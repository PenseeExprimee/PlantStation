#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include "plant/IWateringStrategy.hpp"
#include "plant/WateringController.hpp"

using namespace plant;

static WateringContext createContext(std::optional<double> humidity, std::optional<double> waterLevel)
{
    WateringContext context;
    context.humidity = humidity;
    context.waterLevel = waterLevel;
    context.hour = 8;
    return context;
}

class FakeStrategy final : public IWateringStrategy
{
public:
    FakeStrategy(WateringDecision decision, int &callCounter) : wateringDecision_(decision), addressCallCounter_(&callCounter) {}
    
    WateringDecision decide(const WateringContext &) const override
    {
        *addressCallCounter_ = *addressCallCounter_ + 1;
        return wateringDecision_;
    }

private:
    WateringDecision wateringDecision_;
    int *addressCallCounter_;
};

class ControllerTest : public ::testing::Test
{
protected:
    double minWaterLevel_ = 20.0;
    WateringContext context_ = createContext(40, 100);
    int callCounter_ = 0;

    WateringController createController(std::unique_ptr<IWateringStrategy> strategy)
    {
        WateringController controller(std::move(strategy), minWaterLevel_);
        return controller;
    }
    int readCounter()
    {
        return callCounter_;
    }
};

TEST_F(ControllerTest, FullTankFollowFakeStrategyOrder)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy));

    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Water);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);
}

TEST_F(ControllerTest, FullTankFakeStrategySaysWaitShouldWait)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Wait, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy));

    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);
}

TEST_F(ControllerTest, EmptyTankFakeStrategySaysWaterShouldWait)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy));
    context_.waterLevel = 7.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
    int counter = readCounter();
    EXPECT_EQ(counter, 0);
}

TEST_F(ControllerTest, NoTankInfoFakeStrategySaysWaterShouldWait)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy));
    context_.waterLevel = std::nullopt;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
    int counter = readCounter();
    EXPECT_EQ(counter, 0);
}

TEST_F(ControllerTest, UnderMinTankLevelFakeStrategySaysWaterShouldWait)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy));
    context_.waterLevel = 19.9;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
    int counter = readCounter();
    EXPECT_EQ(counter, 0);
}

TEST_F(ControllerTest, TankLevelMinimumFakeStrategySaysWaterShouldWater)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy));
    context_.waterLevel = 20.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Water);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);
}

TEST_F(ControllerTest, ChangeStrategyFollowNewStrategy)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy));
    
    context_.waterLevel = 25.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Water);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);

    int secondCallCounter = 0;
    std::unique_ptr<FakeStrategy> fakeStrategyTwo= std::make_unique<FakeStrategy>(WateringDecision::Wait, secondCallCounter);
    controller.changeStrategy(std::move(fakeStrategyTwo));
    WateringDecision decisionTwo = controller.decide(context_);

    EXPECT_EQ(decisionTwo, WateringDecision::Wait);
    int counterTwo = readCounter();
    EXPECT_EQ(secondCallCounter, 1);
    EXPECT_EQ(counterTwo, 1);
}

TEST_F(ControllerTest, NoStrategyShouldWait)
{
    std::unique_ptr<IWateringStrategy> noPointer;
    WateringController controller = createController(std::move(noPointer));
    
    context_.waterLevel = 25.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
}

TEST_F(ControllerTest, ChangeStrategyToNullPonterShouldKeepOldOne)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy));
    
    context_.waterLevel = 25.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Water);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);

    std::unique_ptr<IWateringStrategy> noPointer;
    controller.changeStrategy(std::move(noPointer));
    WateringDecision decisionTwo = controller.decide(context_);

    EXPECT_EQ(decisionTwo, WateringDecision::Water);
    int counterTwo = readCounter();
    EXPECT_EQ(counterTwo, 2);
}

TEST_F(ControllerTest, ControllerBuiltWithNoStrategyFollowsNewOne)
{
    std::unique_ptr<IWateringStrategy> noPointer;
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(noPointer));
    
    context_.waterLevel = 25.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);

    controller.changeStrategy(std::move(fakeStrategy));
    WateringDecision decisionTwo = controller.decide(context_);

    EXPECT_EQ(decisionTwo, WateringDecision::Water);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);
}