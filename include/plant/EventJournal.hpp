#pragma once

#include <optional>
#include "plant/RingBuffer.hpp"
#include "plant/SubscribeHandler.hpp"

namespace plant {
    template <std::size_t N>
    class EventJournal {
        public:
            void save(WateringEvent event){
                events_.add(event);
            }

            std::size_t size() const {
                return events_.size();
            }

            std::optional<WateringEvent> at(std::size_t index) const{
                return events_.at(index);
            }

        private:
            RingBuffer<WateringEvent, N> events_;

    };
    
}