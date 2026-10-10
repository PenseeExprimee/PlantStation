#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include "plant/IWateringStrategy.hpp"
#include "plant/WateringThreshold.hpp"

using namespace plant;

static WateringContext createContext(std::optional<double> humidity){
    WateringContext context;
    context.humidity = humidity;
    return context;
}

TEST(WateringStrategy, HumidityUnderThresholdWatering){
    double threshold = 30;
    WateringContext context = createContext(12);
    
    std::unique_ptr<IWateringStrategy> strategy = std::make_unique<WateringThreshold>(threshold);
    
    WateringDecision decision = strategy->decide(context);

    EXPECT_EQ(decision, WateringDecision::Water);

}

TEST(WateringStrategy, HumidityOverThresholdWait){
    double threshold = 36;
    WateringContext context = createContext(42);
    std::unique_ptr<IWateringStrategy> strategy = std::make_unique<WateringThreshold>(threshold);
    
    WateringDecision decision = strategy->decide(context);

    EXPECT_EQ(decision, WateringDecision::Wait);

}

TEST(WateringStrategy, HumidityEqualThresholdWait){
    double threshold = 36;
    WateringContext context = createContext(36);
    std::unique_ptr<IWateringStrategy> strategy = std::make_unique<WateringThreshold>(threshold);
    
    WateringDecision decision = strategy->decide(context);

    EXPECT_EQ(decision, WateringDecision::Wait);

}

TEST(WateringStrategy, NoHumidityWait){
    double threshold = 37;
    WateringContext context = createContext(std::nullopt);
    std::unique_ptr<IWateringStrategy> strategy = std::make_unique<WateringThreshold>(threshold);
    
    WateringDecision decision = strategy->decide(context);

    EXPECT_EQ(decision, WateringDecision::Wait);

}

TEST(WateringStrategy, DifferentThresholdDifferentDecisions){
    double thresholdOne = 37;
    double thresholdTwo = 96;
    WateringContext context = createContext(42);

    std::unique_ptr<IWateringStrategy> strategyOne = std::make_unique<WateringThreshold>(thresholdOne);
    std::unique_ptr<IWateringStrategy> strategyTwo = std::make_unique<WateringThreshold>(thresholdTwo);
    
    WateringDecision decisionOne = strategyOne->decide(context);
    WateringDecision decisionTwo = strategyTwo->decide(context);

    EXPECT_EQ(decisionOne, WateringDecision::Wait);
    EXPECT_EQ(decisionTwo, WateringDecision::Water);

}