#pragma once
#include <algorithm>
#include <vector>
#include <functional>
#include "plant/WateringEnum.hpp"

namespace plant
{
    class SubscribeHandler final
    {
    public:
        void subscribe(std::function<void(WateringEvent)> newSubscriber)
        {
            subscribers_.push_back(std::move(newSubscriber));
        }

        void diffuse(WateringEvent event) const
        {
            std::for_each(
                subscribers_.begin(),
                subscribers_.end(),
                [&event]( const std::function<void(WateringEvent)> &subscriber)
                {
                    subscriber(event);
                });
        }

    private:
        std::vector<std::function<void(WateringEvent)>> subscribers_;
    };
}