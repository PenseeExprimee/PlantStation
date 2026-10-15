#include <iostream>
#include <vector>
#include <optional>
#include <memory>
#include "plant/ISensor.hpp"
#include "plant/FakeSensor.hpp"
#include "plant/MovingAverage.hpp"
#include "plant/EventJournal.hpp"
#include "plant/Alarm.hpp"
#include "plant/IWateringStrategy.hpp"
#include "plant/StrategyFactory.hpp"
#include "plant/WateringController.hpp"

using namespace plant;

constexpr double HUMIDITY_WATERING_THRESHOLD = 30;
constexpr double MINIMUM_WATER_LEVEL = 20;
constexpr std::size_t MOVING_AVERAGE_WINDOW_SIZE = 3;
constexpr std::size_t ALARM_TRIGGER_THRESHOLD = 3;
constexpr std::size_t JOURNAL_CAPACITY = 8;
constexpr std::size_t NUMBER_OF_CYCLES = 16;

int main()
{
    Configuration configuration = {HUMIDITY_WATERING_THRESHOLD, ConfigurationType::Threshold};

    std::vector<std::optional<double>> humidityScript = {
        20, 25, 45, 50, 15, 10, 18, 12, 15, 60, 55, 20, 10, 10, 5, 8};

    std::vector<std::optional<double>> tankScript = {
        80, 75, 70, 65, 60, 55, 10, 5, 90, 85, 80, std::nullopt, std::nullopt,
        std::nullopt, 65, 60};
    
    if (humidityScript.size() != NUMBER_OF_CYCLES || tankScript.size() != NUMBER_OF_CYCLES){
        std::cerr << "The size of one or both of the scripts is incorrect." << "\n";
        return 1;
    }
    std::unique_ptr<ISensor> humiditySensor = std::make_unique<FakeSensor>("humiditySensor", humidityScript);
    std::unique_ptr<ISensor> tankLevelSensor = std::make_unique<FakeSensor>("tankLevelSensor", tankScript);

    MovingAverage<double, MOVING_AVERAGE_WINDOW_SIZE> humidityFilter;
    EventJournal<JOURNAL_CAPACITY> eventJournal;
    Alarm<ALARM_TRIGGER_THRESHOLD> anomalyAlarm;
    
    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    if (strategy == nullptr){
        std::cerr << "The factory could not create a strategy." << "\n";
        return 1;
    }
    WateringController controller(std::move(strategy), MINIMUM_WATER_LEVEL);

    return 0;
}