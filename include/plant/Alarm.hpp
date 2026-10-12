#pragma once
#include <cstddef>
#include "WateringEnum.hpp"

namespace plant
{
    template <std::size_t N>
    class Alarm final
    {
        static_assert(N > 0, "The alarm threshold must be greater than zero");

    public:
        void handle(WateringEvent event)
        {
            if (event == WateringEvent::StrategyMissing || event == WateringEvent::UnknownTank || event == WateringEvent::LowWaterLevel)
            {
                alarmCounter_++;
            }
            else
            {
                alarmCounter_ = 0;
            }
            raise();
        }

        bool isRaised() const
        {
            return alarmIsRaised_;
        }

        std::size_t consecutiveAnomalies() const
        {
            return alarmCounter_;
        }
        void acknowledge()
        {
            if (alarmIsRaised_)
            {
                alarmIsRaised_ = false;
                alarmCounter_ = 0;
            }
        }

    private:
        bool alarmIsRaised_ = false;
        std::size_t alarmCounter_ = 0;

        void raise()
        {
            if (alarmCounter_ >= N)
            {
                alarmIsRaised_ = true;
            }
        }
    };
}