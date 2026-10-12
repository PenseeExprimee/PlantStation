#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <vector>
#include <utility>
#include "plant/IWateringStrategy.hpp"
#include "plant/WateringController.hpp"
#include "plant/WateringEnum.hpp"
#include "utils/test_helpers.hpp"

using namespace plant;


class ControllerTest : public ::testing::Test
{
protected:
    double minWaterLevel_ = 20.0;
    WateringContext context_ = createContext(40, 100);
    int callCounter_ = 0;
    int callCounterTwo_ = 0;
    std::vector<WateringEvent> eventsReceived_ = {};
    std::vector<WateringEvent> eventsReceivedTwo_ = {};

    int readCounter()
    {
        return callCounter_;
    }

    void subscribeRecorder(WateringController &controller, std::vector<WateringEvent> &recorder){
        auto getEvent = [&recorder](WateringEvent event){
            recorder.push_back(event);
        };
        controller.subscribe(getEvent);
    }
};

TEST_F(ControllerTest, FullTankFollowFakeStrategyOrder)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);

    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Water);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);
}

TEST_F(ControllerTest, FullTankFakeStrategySaysWaitShouldWait)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Wait, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);

    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);
}

TEST_F(ControllerTest, EmptyTankFakeStrategySaysWaterShouldWait)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);
    context_.waterLevel = 7.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
    int counter = readCounter();
    EXPECT_EQ(counter, 0);
}

TEST_F(ControllerTest, NoTankInfoFakeStrategySaysWaterShouldWait)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);
    context_.waterLevel = std::nullopt;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
    int counter = readCounter();
    EXPECT_EQ(counter, 0);
}

TEST_F(ControllerTest, UnderMinTankLevelFakeStrategySaysWaterShouldWait)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);
    context_.waterLevel = 19.9;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
    int counter = readCounter();
    EXPECT_EQ(counter, 0);
}

TEST_F(ControllerTest, TankLevelMinimumFakeStrategySaysWaterShouldWater)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy),minWaterLevel_);
    context_.waterLevel = 20.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Water);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);
}

TEST_F(ControllerTest, ChangeStrategyFollowNewStrategy)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);
    
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
    counter = readCounter();
    EXPECT_EQ(secondCallCounter, 1);
    EXPECT_EQ(counter, 1);
}

TEST_F(ControllerTest, NoStrategyShouldWait)
{
    std::unique_ptr<IWateringStrategy> noPointer;
    WateringController controller = createController(std::move(noPointer), minWaterLevel_);
    
    context_.waterLevel = 25.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
}

TEST_F(ControllerTest, ChangeStrategyToNullPointerShouldKeepOldOne)
{
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);
    
    context_.waterLevel = 25.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Water);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);

    std::unique_ptr<IWateringStrategy> noPointer;
    controller.changeStrategy(std::move(noPointer));
    WateringDecision decisionTwo = controller.decide(context_);

    EXPECT_EQ(decisionTwo, WateringDecision::Water);
    counter = readCounter();
    EXPECT_EQ(counter, 2);
}

TEST_F(ControllerTest, ControllerBuiltWithNoStrategyFollowsNewOne)
{
    std::unique_ptr<IWateringStrategy> noPointer;
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    WateringController controller = createController(std::move(noPointer), minWaterLevel_);
    
    context_.waterLevel = 25.0;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);

    controller.changeStrategy(std::move(fakeStrategy));
    WateringDecision decisionTwo = controller.decide(context_);

    EXPECT_EQ(decisionTwo, WateringDecision::Water);
    int counter = readCounter();
    EXPECT_EQ(counter, 1);
}

TEST_F(ControllerTest, NoStrategyEventReceivedStrategyMissing){
    std::unique_ptr<IWateringStrategy> noPointer;

    std::vector<WateringEvent> expectedEventsList = {WateringEvent::StrategyMissing};

    WateringController controller = createController(std::move(noPointer), minWaterLevel_);
    subscribeRecorder(controller, eventsReceived_);
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(eventsReceived_, expectedEventsList);
    EXPECT_EQ(decision, WateringDecision::Wait);
}

TEST_F(ControllerTest, MissingTankEventReceivedUnknownTank){
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    std::vector<WateringEvent> expectedEventsList = {WateringEvent::UnknownTank};
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);

    subscribeRecorder(controller, eventsReceived_);
    context_.waterLevel = std::nullopt;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(eventsReceived_, expectedEventsList);
    EXPECT_EQ(decision, WateringDecision::Wait);
    EXPECT_EQ(callCounter_, 0);
}

TEST_F(ControllerTest, LowWaterLevelEventReceivedLowWaterLevel){
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);

    std::vector<WateringEvent> expectedEventsList = {WateringEvent::LowWaterLevel};
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);
    subscribeRecorder(controller, eventsReceived_);

    context_.waterLevel = 19.9;
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(eventsReceived_, expectedEventsList);
    EXPECT_EQ(decision, WateringDecision::Wait);
    EXPECT_EQ(callCounter_, 0);
}

TEST_F(ControllerTest, StrategySaysWaterEventReceivedStrategySaysWater){
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);

    std::vector<WateringEvent> expectedEventsList = {WateringEvent::StrategyDecisionWater};

    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);
    subscribeRecorder(controller, eventsReceived_);
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(eventsReceived_, expectedEventsList);
    EXPECT_EQ(callCounter_, 1);
    EXPECT_EQ(decision, WateringDecision::Water);
}

TEST_F(ControllerTest, StrategySaysWaitEventReceivedStrategySaysWait){
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Wait, callCounter_);

    std::vector<WateringEvent> expectedEventsList = {WateringEvent::StrategyDecisionWait};

    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);
    subscribeRecorder(controller, eventsReceived_);
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(eventsReceived_, expectedEventsList);
    EXPECT_EQ(callCounter_, 1);
    EXPECT_EQ(decision, WateringDecision::Wait);
}

TEST_F(ControllerTest, ManyEventsReceivedInOrder){
    std::unique_ptr<FakeStrategy> fakeStrategyOne = std::make_unique<FakeStrategy>(WateringDecision::Wait, callCounter_);
    std::unique_ptr<FakeStrategy> fakeStrategyTwo = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounterTwo_);

    std::vector<WateringEvent> expectedEventsListOne = {WateringEvent::StrategyDecisionWait};
    std::vector<WateringEvent> expectedEventsListTwo = {WateringEvent::StrategyDecisionWait, WateringEvent::StrategyDecisionWater};

    WateringController controller = createController(std::move(fakeStrategyOne), minWaterLevel_);
    subscribeRecorder(controller, eventsReceived_);
    WateringDecision decision = controller.decide(context_);

    EXPECT_EQ(eventsReceived_, expectedEventsListOne);
    EXPECT_EQ(callCounter_, 1);
    EXPECT_EQ(decision, WateringDecision::Wait);

    controller.changeStrategy(std::move(fakeStrategyTwo));
    decision = controller.decide(context_);
    EXPECT_EQ(eventsReceived_, expectedEventsListTwo);
    EXPECT_EQ(callCounterTwo_, 1);
    EXPECT_EQ(decision, WateringDecision::Water);
}

TEST_F(ControllerTest, TwoSubscribersOneControllerReceiveTheSameEvent){
    std::unique_ptr<FakeStrategy> fakeStrategyOne = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    std::vector<WateringEvent> expectedEventsList = {WateringEvent::StrategyDecisionWater};

    WateringController controller = createController(std::move(fakeStrategyOne), minWaterLevel_);
    
    subscribeRecorder(controller, eventsReceived_);
    subscribeRecorder(controller, eventsReceivedTwo_);
    
    WateringDecision decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Water);
    EXPECT_EQ(eventsReceived_, expectedEventsList);
    EXPECT_EQ(eventsReceivedTwo_, expectedEventsList);
    EXPECT_EQ(callCounter_, 1);

}

TEST_F(ControllerTest, LateSubscriberDoesNotGetPreviousEvent){
    std::unique_ptr<FakeStrategy> fakeStrategyOne = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);
    std::vector<WateringEvent> expectedEventsList = {WateringEvent::StrategyDecisionWater, WateringEvent::LowWaterLevel};
    std::vector<WateringEvent> expectedEventsListTwo = {WateringEvent::LowWaterLevel};

    WateringController controller = createController(std::move(fakeStrategyOne), minWaterLevel_);
    subscribeRecorder(controller, eventsReceived_);
    WateringDecision decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Water);

    subscribeRecorder(controller, eventsReceivedTwo_);
    context_.waterLevel = 3.0;
    decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Wait);
    EXPECT_EQ(eventsReceived_, expectedEventsList);
    EXPECT_EQ(eventsReceivedTwo_, expectedEventsListTwo);
    EXPECT_EQ(callCounter_, 1);
}