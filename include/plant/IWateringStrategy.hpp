#pragma once
#include <optional>

namespace plant
{
    struct WateringContext
    {
        std::optional<double> humidity;
        std::optional<double> waterLevel;
        int hour = 0;
    };

    enum class WateringDecision
    {
        Water,
        Wait
    };

    class IWateringStrategy
    {
    public:
        virtual WateringDecision decide(const WateringContext&measurements) const = 0;
        virtual ~IWateringStrategy() = default;
    };

}