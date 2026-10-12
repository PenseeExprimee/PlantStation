#pragma once
#include <optional>
#include <utility>
#include <memory>
#include "plant/IWateringStrategy.hpp"
#include "plant/WateringController.hpp"
#include "plant/Alarm.hpp"
#include "plant/EventJournal.hpp"

namespace plant
{
    inline WateringContext createContext(std::optional<double> humidity, std::optional<double> waterLevel)
    {
        WateringContext context;
        context.humidity = humidity;
        context.waterLevel = waterLevel;
        context.hour = 8;
        return context;
    }

    class FakeStrategy final : public IWateringStrategy
    {
    public:
        FakeStrategy(WateringDecision decision, int &callCounter) : wateringDecision_(decision), addressCallCounter_(&callCounter) {}

        WateringDecision decide(const WateringContext &) const override
        {
            *addressCallCounter_ = *addressCallCounter_ + 1;
            return wateringDecision_;
        }

    private:
        WateringDecision wateringDecision_;
        int *addressCallCounter_;
    };

    inline WateringController createController(std::unique_ptr<IWateringStrategy> strategy, const double minWaterLevel)
    {
        WateringController controller(std::move(strategy), minWaterLevel);
        return controller;
    }

    template <std::size_t N>
    void subscribeAlarm(WateringController &controller, Alarm<N> &alarm){
        auto handle = [&alarm](WateringEvent event){
            alarm.handle(event);
        };
        controller.subscribe(handle);
    }

    template <std::size_t N>
    void subscribeJournal(WateringController &controller, EventJournal<N> &journal){
        auto getEvent = [&journal](WateringEvent event){
            journal.save(event);
        };
        controller.subscribe(getEvent);
    }
}