#include <iostream>
#include <vector>
#include <optional>
#include <memory>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <string_view>
#include "plant/ISensor.hpp"
#include "plant/FakeSensor.hpp"
#include "plant/MovingAverage.hpp"
#include "plant/EventJournal.hpp"
#include "plant/Alarm.hpp"
#include "plant/IWateringStrategy.hpp"
#include "plant/StrategyFactory.hpp"
#include "plant/WateringController.hpp"
#include "plant/EventText.hpp"

using namespace plant;

constexpr double HUMIDITY_WATERING_THRESHOLD = 30;
constexpr double MINIMUM_WATER_LEVEL = 20;
constexpr std::size_t MOVING_AVERAGE_WINDOW_SIZE = 3;
constexpr std::size_t ALARM_TRIGGER_THRESHOLD = 3;
constexpr std::size_t JOURNAL_CAPACITY = 8;
constexpr std::size_t NUMBER_OF_CYCLES = 16;
constexpr std::size_t EXPECTED_FIRST_ALARM_CYCLE = 14;

static_assert(
    JOURNAL_CAPACITY <= NUMBER_OF_CYCLES,
    "JOURNAL_CAPACITY must be <= NUMBER_OF_CYCLES"
);

static_assert(
    EXPECTED_FIRST_ALARM_CYCLE <= NUMBER_OF_CYCLES,
    "EXPECTED_FIRST_ALARM_CYCLE must be <= NUMBER_OF_CYCLES"
);
constexpr std::size_t FORGOTTEN_CYCLES = NUMBER_OF_CYCLES - JOURNAL_CAPACITY;

static std::string optionalToString(const std::optional<double>& value)
{
    if (!value.has_value())
    {
        return "n/a";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << *value;
    return oss.str();
}

static std::string_view alarmStateText(bool alarmStatus) {
    if (alarmStatus){
        return "raised";
    }
    else {
        return "idle";
    }
}

int main()
{
    Configuration configuration = {HUMIDITY_WATERING_THRESHOLD, ConfigurationType::Threshold};
    std::vector<WateringEvent> expectedEvents = {
        WateringEvent::StrategyDecisionWater,
        WateringEvent::StrategyDecisionWater,
        WateringEvent::StrategyDecisionWait,
        WateringEvent::StrategyDecisionWait,
        WateringEvent::StrategyDecisionWait,
        WateringEvent::StrategyDecisionWater,
        WateringEvent::LowWaterLevel,
        WateringEvent::LowWaterLevel,
        WateringEvent::StrategyDecisionWater,
        WateringEvent::StrategyDecisionWater,
        WateringEvent::StrategyDecisionWait,
        WateringEvent::UnknownTank,
        WateringEvent::UnknownTank,
        WateringEvent::UnknownTank,
        WateringEvent::StrategyDecisionWater,
        WateringEvent::StrategyDecisionWater
    };
    std::vector<WateringEvent> eventHistory = {};
    std::vector<std::optional<double>> humidityScript = {
        20, 25, 45, 50, 15, 10, 18, 12, 15, 60, 55, 20, 10, 10, 5, 8};

    std::vector<std::optional<double>> tankScript = {
        80, 75, 70, 65, 60, 55, 10, 5, 90, 85, 80, std::nullopt, std::nullopt,
        std::nullopt, 65, 60};

    if (humidityScript.size() != NUMBER_OF_CYCLES || tankScript.size() != NUMBER_OF_CYCLES || expectedEvents.size() != NUMBER_OF_CYCLES)
    {
        std::cerr << "The size of one or both of the scripts is incorrect." << "\n";
        return EXIT_FAILURE;
    }
    std::unique_ptr<ISensor> humiditySensor = std::make_unique<FakeSensor>("humiditySensor", humidityScript);
    std::unique_ptr<ISensor> tankLevelSensor = std::make_unique<FakeSensor>("tankLevelSensor", tankScript);

    MovingAverage<double, MOVING_AVERAGE_WINDOW_SIZE> humidityFilter;
    EventJournal<JOURNAL_CAPACITY> eventJournal;
    Alarm<ALARM_TRIGGER_THRESHOLD> anomalyAlarm;

    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    if (strategy == nullptr)
    {
        std::cerr << "The factory could not create a strategy." << "\n";
        return EXIT_FAILURE;
    }

    std::optional<WateringEvent> lastEvent;
    WateringController controller(std::move(strategy), MINIMUM_WATER_LEVEL);

    // journal
    controller.subscribe([&eventJournal](WateringEvent event)
                         { eventJournal.save(event); });
    // alarm
    controller.subscribe([&anomalyAlarm](WateringEvent event)
                         { anomalyAlarm.handle(event); });
    // display
    controller.subscribe([&lastEvent](WateringEvent event)
                         { lastEvent = event; });

    //LOOP
    std::cout << std::left
          << std::setw(8)  << "Cycle"
          << std::setw(18) << "Read humidity"
          << std::setw(16) << "Tank level"
          << std::setw(22) << "Average humidity"
          << std::setw(17) << "Alarm counter"
          << std::setw(16) << "Alarm state"
          << "Event text"
          << '\n';

    std::cout << std::string(97, '-') << '\n';
    std::optional<std::size_t> firstAlarmRaise;
    for (std::size_t i = 0; i < NUMBER_OF_CYCLES; i++){

        //Humidity sensor
        std::optional<double> humidityValue = humiditySensor->read();
        if (humidityValue.has_value()){
            humidityFilter.add(*humidityValue);
        }
        std::optional<double> humidityAverage = humidityFilter.average();

        //Tank level sensor
        std::optional<double> tankLevel = tankLevelSensor -> read();

        //Context creation, the hour is not simulated
        WateringContext context = {humidityAverage, tankLevel};

        //reset
        lastEvent.reset();

        //Decision
        controller.decide(context);

        //Event
        if (!lastEvent.has_value()){
            std::cerr << "The controller could not determine the event." << "\n";
            return EXIT_FAILURE;
        }

        //Store the lastEvent
        eventHistory.push_back(*lastEvent);

        //Register the first alarm raise
        if (!firstAlarmRaise.has_value() && anomalyAlarm.isRaised()){
            firstAlarmRaise = i + 1;
        }
        
        //Display message
        std::cout
        << std::setw(8) << i + 1
        << std::setw(18) << optionalToString(humidityValue) 
        << std::setw(16) << optionalToString(tankLevel)
        << std::setw(22) << optionalToString(humidityAverage)
        << std::setw(17) << anomalyAlarm.consecutiveAnomalies()
        << std::setw(16) << alarmStateText(anomalyAlarm.isRaised())
        << eventText(*lastEvent) << "\n";
    }

    //Events stored in the journal
    std::cout << "--------------JOURNAL------------------" << "\n";
    for (std::size_t j = 0; j < eventJournal.size(); j++){
        std::cout << eventText(*eventJournal.at(j)) << "\n";  
    }

    //Alarm status
    std::cout << "Alarm status: " << alarmStateText(anomalyAlarm.isRaised()) << "\n";
    std::cout << "Anomalies counter: " << anomalyAlarm.consecutiveAnomalies() << "\n";
    if (!anomalyAlarm.isRaised()){
        std::cerr << "The alarm should be raised before acknowledgment." << "\n";
        return EXIT_FAILURE;
    }

    //Acknowledgment
    anomalyAlarm.acknowledge();

    //Re check alarm status
    std::cout << "Alarm status: " << alarmStateText(anomalyAlarm.isRaised()) << "\n";
    std::cout << "Anomalies counter: " << anomalyAlarm.consecutiveAnomalies() << "\n";
    if (anomalyAlarm.isRaised()){
        std::cerr << "The alarm should NOT be raised after acknowledgment." << "\n";
        return EXIT_FAILURE;
    }
    
    //AUTO VERIFICATION
    if (eventJournal.size() != JOURNAL_CAPACITY){
        std::cerr << "An error occurred with the journal. Expected size: " << JOURNAL_CAPACITY << " Actual size: " << eventJournal.size() << "\n";
        return EXIT_FAILURE;
    }

    for (std::size_t i = 0; i < JOURNAL_CAPACITY; i++){
        if (!(eventJournal.at(i) == expectedEvents[i + FORGOTTEN_CYCLES])){
            std::cerr << "The event " << i + 1 << " in the journal is incorrect." << "\n";
            return EXIT_FAILURE;
        }
    }

    //Also check the first events that are no longer in the journal
    for (std::size_t i = 0; i < FORGOTTEN_CYCLES; i ++){
        if(!(eventHistory.at(i) == expectedEvents[i])){
            std::cerr << "The event " << i + 1 << " is incorrect." << "\n";
            return EXIT_FAILURE;
        }
    }
    if (firstAlarmRaise != EXPECTED_FIRST_ALARM_CYCLE){
        std::cerr << "The alarm should be raised at the "<< EXPECTED_FIRST_ALARM_CYCLE << "th step." << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}