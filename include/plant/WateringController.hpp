#pragma once
#include <memory>
#include <optional>
#include <utility>
#include <functional>
#include "plant/IWateringStrategy.hpp"
#include "plant/SubscribeHandler.hpp"
#include "plant/WateringEnum.hpp"

namespace plant
{
    class WateringController final
    {
    public:

        explicit WateringController(std::unique_ptr<IWateringStrategy> strategy, const double minWaterLevel) : strategy_(std::move(strategy)), minWaterLevel_(minWaterLevel) {}

        WateringDecision decide(const WateringContext &context) const
        {
            DecisionOutcome decisionReason;
            calculateDecisionAndReason(context, decisionReason);
            subscribers_.diffuse(decisionReason.event);
            return decisionReason.decision;
        }

        void changeStrategy(std::unique_ptr<IWateringStrategy> newStrategy)
        {
            if (newStrategy != nullptr)
            {
                strategy_ = std::move(newStrategy);
            }
        }

        void subscribe(std::function<void(WateringEvent)> newSubscriber)
        {
            subscribers_.subscribe(std::move(newSubscriber));
        }

    private:
        struct DecisionOutcome
        {
            WateringDecision decision = WateringDecision::Wait;
            WateringEvent event = WateringEvent::StrategyDecisionWait;
        };
        std::unique_ptr<IWateringStrategy> strategy_;
        const double minWaterLevel_;
        SubscribeHandler subscribers_;

        void calculateDecisionAndReason(const WateringContext& context, DecisionOutcome& decisionReason) const {
            if (strategy_ == nullptr)
            {
                decisionReason.decision = WateringDecision::Wait;
                decisionReason.event = WateringEvent::StrategyMissing;
            }
            else if (!context.waterLevel.has_value())
            {
                decisionReason.decision = WateringDecision::Wait;
                decisionReason.event = WateringEvent::UnknownTank;
            }
            else if (context.waterLevel < minWaterLevel_)
            {
                decisionReason.decision = WateringDecision::Wait;
                decisionReason.event = WateringEvent::LowWaterLevel;
            }
            else {
                decisionReason.decision = strategy_->decide(context);

                if(decisionReason.decision == WateringDecision::Wait){
                    decisionReason.event = WateringEvent::StrategyDecisionWait;
                }
                else{
                    decisionReason.event = WateringEvent::StrategyDecisionWater;
                }
            }
        }
    };
}