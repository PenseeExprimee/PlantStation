#pragma once
#include <memory>
#include <optional>
#include "plant/IWateringStrategy.hpp"
#include <utility>

namespace plant {
    class WateringController final {
        public:
            explicit WateringController(std::unique_ptr<IWateringStrategy> strategy, const double minWaterLevel): strategy_(std::move(strategy)), minWaterLevel_(minWaterLevel){}

            WateringDecision decide(const WateringContext &context) const{
                if(strategy_ == nullptr){
                    return WateringDecision::Wait;
                }
                if(!context.waterLevel.has_value()){
                    return WateringDecision::Wait;
                }
                if (context.waterLevel < minWaterLevel_){
                    return WateringDecision::Wait;
                }
                return strategy_ -> decide(context);
            }

            void changeStrategy(std::unique_ptr<IWateringStrategy> newStrategy){
                if (newStrategy != nullptr){
                    strategy_ = std::move(newStrategy);
                }
            }
        private:
            std::unique_ptr<IWateringStrategy> strategy_;
            const double minWaterLevel_;
    };
}