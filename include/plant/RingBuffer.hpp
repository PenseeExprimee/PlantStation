#pragma once
#include <array>
#include <optional>
#include <cstddef>

namespace plant
{
    /*RingBuffer is a fixed-size circular buffer that stores the most recent measurements of type `T`.
    When the buffer is full, new measurements overwrite the oldest ones.
    It provides methods to check its size and state (empty or full), and to access stored measurements in chronological order, from oldest to newest.
    Invalid indices return `std::nullopt`.
    */
    template <typename T, std::size_t N>
    class RingBuffer
    {
        static_assert(N > 0, "RingBuffer size must be greater than zero");

    public:
        void add(const T &measure)
        {
            data_[writingIndex_] = measure;
            if (writtenElements_ < N)
            {
                writtenElements_++;
            }
            writingIndex_ = (writingIndex_ + 1) % N;
        }
        std::size_t size() const
        {
            return writtenElements_;
        }
        bool empty() const
        {
            return (writtenElements_ == 0);
        }

        bool full() const
        {
            return (writtenElements_ == N);
        }
        std::optional<T> at(std::size_t index) const
        {
            if (index >= writtenElements_)
            {
                return std::nullopt;
            }
            std::size_t oldestIndex = (N + writingIndex_ - writtenElements_);
            std::size_t indexResult = (oldestIndex + index) % N;
            return data_[indexResult];
        }

    private:
        std::array<T, N> data_ = {};
        std::size_t writingIndex_ = 0;
        std::size_t writtenElements_ = 0;
    };
}