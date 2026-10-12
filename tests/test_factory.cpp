#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <limits>
#include "plant/IWateringStrategy.hpp"
#include "plant/WateringController.hpp"
#include "plant/StrategyFactory.hpp"
#include "utils/test_helpers.hpp"

using namespace plant;

constexpr double POS_INF = std::numeric_limits<double>::infinity();
constexpr double NEG_INF = -std::numeric_limits<double>::infinity();


class FactoryTestIntegration : public ::testing::Test
{
protected:
    double minWaterLevel_ = 20.0;
    WateringContext context_ = createContext(12, 100);
};

TEST(FactoryTest, ValidConfiguration){

    Configuration configuration = {10, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    ASSERT_NE(strategy, nullptr);
}

TEST(FactoryTest, ThresholdUnderMinReturnsNull){
    Configuration configuration = {-0.1, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    ASSERT_EQ(strategy, nullptr);
}

TEST(FactoryTest, ThresholdOverMaxReturnsNull){
    Configuration configuration = {100.1, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    ASSERT_EQ(strategy, nullptr);
}

TEST(FactoryTest, UpperBoundValid){
    Configuration configuration = {100, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    ASSERT_NE(strategy, nullptr);
}

TEST(FactoryTest, LowerBoundValid){
    Configuration configurationTwo = {0, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategyTwo = createStrategy(configurationTwo);
    ASSERT_NE(strategyTwo, nullptr);
}

TEST(FactoryTest, NanConfiguration){
    auto isANan = std::numeric_limits<double>::quiet_NaN();
    Configuration configuration = {isANan, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    EXPECT_EQ(strategy, nullptr);
}

TEST_F(FactoryTestIntegration, StrategyIntegrationWorksFine){
    Configuration configuration = {14, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    ASSERT_NE(strategy, nullptr);

    WateringController wateringController = createController(std::move(strategy), minWaterLevel_);
    WateringDecision decision = wateringController.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Water);

    Configuration configurationTwo = {10, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategyTwo = createStrategy(configurationTwo);
    ASSERT_NE(strategyTwo, nullptr);

    WateringController wateringControllerTwo = createController(std::move(strategyTwo), minWaterLevel_);
    WateringDecision decisionTwo = wateringControllerTwo.decide(context_);

    EXPECT_EQ(decisionTwo, WateringDecision::Wait);
}

TEST(FactoryTest,InfinityPlusRejected){
    double maxValue = POS_INF;
    Configuration configuration = {maxValue, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    ASSERT_EQ(strategy, nullptr);

}

TEST(FactoryTest,InfinityMinusRejected){
    double maxValue = NEG_INF;
    Configuration configuration = {maxValue, ConfigurationType::Threshold};
    std::unique_ptr<IWateringStrategy> strategy = createStrategy(configuration);
    ASSERT_EQ(strategy, nullptr);
}