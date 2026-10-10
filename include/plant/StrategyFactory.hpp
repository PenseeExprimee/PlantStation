#pragma once
#include <memory>
#include <cmath>
#include "plant/IWateringStrategy.hpp"
#include "plant/WateringThreshold.hpp"

namespace plant
{
    constexpr double MIN_VALID_THRESHOLD = 0.0;
    constexpr double MAX_VALID_THRESHOLD = 100.0;

    enum class ConfigurationType
    {
        Threshold
    };

    struct Configuration
    {
        double threshold;
        ConfigurationType configurationType;
    };

    inline std::unique_ptr<IWateringStrategy> createStrategy(const Configuration &configuration)
    {
        // Check that the threshold is valid
        if (std::isnan(configuration.threshold) || configuration.threshold > MAX_VALID_THRESHOLD || configuration.threshold < MIN_VALID_THRESHOLD )
        {
            return nullptr;
        }
        switch (configuration.configurationType)
        {
        case ConfigurationType::Threshold:
            return std::make_unique<WateringThreshold>(configuration.threshold);
        }
        return nullptr;
    }

}