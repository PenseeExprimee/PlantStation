#pragma once
#include <string_view>
#include "plant/WateringEnum.hpp"

namespace plant
{
    inline std::string_view eventText(WateringEvent event)
    {
        switch (event)
        {
        case WateringEvent::StrategyMissing:
            return "The strategy is missing.";
        case WateringEvent::UnknownTank:
            return "The level of the tank cannot be read.";
        case WateringEvent::LowWaterLevel:
            return "There is not enough water left in the tank.";
        case WateringEvent::StrategyDecisionWait:
            return "The plant does not need to be watered right now. Let's wait.";
        case WateringEvent::StrategyDecisionWater:
            return "The plant needs watering.";
        }
        return "Unknown event.";
    }
}