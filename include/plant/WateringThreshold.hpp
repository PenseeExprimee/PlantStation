#pragma once
#include <optional>
#include "IWateringStrategy.hpp"

namespace plant {
class WateringThreshold final : public IWateringStrategy
    {
    public:
        explicit WateringThreshold(double threshold) : threshold_(threshold) {}

        WateringDecision decide(const WateringContext &context) const override
        {
            if(!context.humidity.has_value()){
                return WateringDecision::Wait;
            }
            if (*context.humidity >= threshold_)
            {
                return WateringDecision::Wait;
            }
            else
            {
                return WateringDecision::Water;
            }
        }

    private:
        const double threshold_;
    };
}