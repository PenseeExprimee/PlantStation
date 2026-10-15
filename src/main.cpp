#include <iostream>
#include <vector>
#include <optional>
#include <memory>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
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

std::string optionalToString(const std::optional<double>& value)
{
    if (!value.has_value())
    {
        return "n/a";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << *value;
    return oss.str();
}

std::string_view alarmStateText(bool alarmStatus) {
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

    std::vector<std::optional<double>> humidityScript = {
        20, 25, 45, 50, 15, 10, 18, 12, 15, 60, 55, 20, 10, 10, 5, 8};

    std::vector<std::optional<double>> tankScript = {
        80, 75, 70, 65, 60, 55, 10, 5, 90, 85, 80, std::nullopt, std::nullopt,
        std::nullopt, 65, 60};

    if (humidityScript.size() != NUMBER_OF_CYCLES || tankScript.size() != NUMBER_OF_CYCLES)
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
        return 1;
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

    std::cout << std::string(115, '-') << '\n';
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

        //Register the first alarm raise
        if (!firstAlarmRaise.has_value() && anomalyAlarm.isRaised()){
            firstAlarmRaise = i;
        }
        //Event
        if (!lastEvent.has_value()){
            std::cerr << "The controller could not determine the event." << "\n";
            return EXIT_FAILURE;
        }
        
        //Display message
        std::cout
        << std::setw(8) << i 
        << std::setw(8) << optionalToString(humidityValue) 
        << std::setw(18) << optionalToString(tankLevel)
        << std::setw(16) << optionalToString(humidityAverage)
        << std::setw(22) << anomalyAlarm.consecutiveAnomalies()
        << std::setw(17) << alarmStateText(anomalyAlarm.isRaised())
        << std::setw(16) << eventText(*lastEvent) << "\n";
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
    if (eventJournal.size() != 8){
        std::cerr << "An error occured with the journal." << std::endl;
        return EXIT_FAILURE;
    }
    if(eventJournal.at(0) != WateringEvent::StrategyDecisionWater){
        std::cerr << "The first event in the journal is incorrect." << std::endl;
        return EXIT_FAILURE;
    }

    if (firstAlarmRaise != 13){
        std::cerr << "The alarm should be raised at the 13th step." << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}