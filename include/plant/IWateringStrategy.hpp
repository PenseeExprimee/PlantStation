#pragma once
#include <optional>
#include "plant/WateringEnum.hpp"

namespace plant
{
    struct WateringContext
    {
        std::optional<double> humidity;
        std::optional<double> waterLevel;
        int hour = 0;
    };

    class IWateringStrategy
    {
    public:
        virtual WateringDecision decide(const WateringContext&measurements) const = 0;
        virtual ~IWateringStrategy() = default;
    };

}