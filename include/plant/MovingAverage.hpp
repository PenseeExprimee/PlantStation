#pragma once
#include <optional>
#include <cstddef>
#include "plant/RingBuffer.hpp"

namespace plant
{

    template <typename T, std::size_t N>
    class MovingAverage
    {
        static_assert(N > 0, "The filter size must be greater than zero");

    public:
        void add(const T &measure)
        {
            ringBuffer_.add(measure);
        }

        std::optional<double> average() const
        {
            // Calculation of the average
            std::size_t writtenElements = ringBuffer_.size();
            if (writtenElements == 0)
            {
                return std::nullopt;
            }
            double total = 0;
            for (std::size_t i = 0; i < writtenElements; i++)
            {
                // i < .size() so the index is valid
                total += *ringBuffer_.at(i);
            }
            return total / writtenElements;
        }

    private:
        RingBuffer<T, N> ringBuffer_;
    };
}