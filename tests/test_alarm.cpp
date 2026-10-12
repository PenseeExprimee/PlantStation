#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <utility>
#include "plant/Alarm.hpp"
#include "plant/WateringEnum.hpp"
#include "plant/IWateringStrategy.hpp"
#include "plant/WateringController.hpp"
#include "utils/test_helpers.hpp"
#include "plant/EventJournal.hpp"

using namespace plant;


class AlarmTest : public ::testing::Test
{
protected:
    double minWaterLevel_ = 20.0;
    WateringContext context_ = createContext(40, 100);
    int callCounter_ = 0;
};

TEST(AlarmTestIndependent, NewAlarmIsNotRaised){
    Alarm<3> alarm;
    bool alarmStatus = alarm.isRaised();
    EXPECT_FALSE(alarmStatus);
}

TEST(AlarmTestIndependent, ThresholdMinus1NoAlarmRaised){
    Alarm<3> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(counter, 2u);
}

TEST(AlarmTestIndependent, EqualThresholdAlarmRaised){
    Alarm<2> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(counter, 2u);
}

TEST(AlarmTestIndependent, OverThresholdAlarmRaised){
    Alarm<3> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::StrategyMissing);
    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(counter, 4u);
}

TEST(AlarmTestIndependent, ConsecutivesAnomaliesInterruptedByNormalDecision){
    Alarm<4> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::StrategyDecisionWater);
    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::LowWaterLevel);
    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(counter, 3u);
}

TEST(AlarmTestIndependent, NormalEventResetCounter){
    Alarm<4> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::StrategyDecisionWater);
    alarm.handle(WateringEvent::LowWaterLevel);

    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(counter, 1u);
}

TEST(AlarmTestIndependent, NAnomaliesThenWaterEventAlarmStillRaised){
    Alarm<4> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::StrategyDecisionWater);

    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(counter, 0u);
}

TEST(AlarmTestIndependent, NAnomaliesThenWaitEventAlarmStillRaised){
    Alarm<4> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::StrategyDecisionWait);

    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(counter, 0u);
}

TEST(AlarmTestIndependent, RaisedAlarmAcknowledgeNoMoreAlarm){
    Alarm<4> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::LowWaterLevel);

    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(counter, 4u);

    alarm.acknowledge();
    alarmStatus = alarm.isRaised();
    counter = alarm.consecutiveAnomalies();
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(counter, 0u);
}

TEST(AlarmTestIndependent, RaisedAlarmAcknowledgeNMinusOneAnomaliesNoRaisedFlag){
    Alarm<4> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::LowWaterLevel);

    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(counter, 4u);

    alarm.acknowledge();
    alarmStatus = alarm.isRaised();
    counter = alarm.consecutiveAnomalies();
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(counter, 0u);

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::UnknownTank);
    alarmStatus = alarm.isRaised();
    counter = alarm.consecutiveAnomalies();
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(counter, 3u);
}

TEST(AlarmTestIndependent, RaisedAlarmAcknowledgeNAnomaliesRaisedFlag){
    Alarm<4> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::LowWaterLevel);

    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(counter, 4u);

    alarm.acknowledge();
    alarmStatus = alarm.isRaised();
    counter = alarm.consecutiveAnomalies();
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(counter, 0u);

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::LowWaterLevel);
    alarmStatus = alarm.isRaised();
    counter = alarm.consecutiveAnomalies();
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(counter, 4u);
}


TEST(AlarmTestIndependent, AcknowledgeOnNonRaisedFlagNoChanges){
    Alarm<4> alarm;

    alarm.handle(WateringEvent::LowWaterLevel);
    alarm.handle(WateringEvent::UnknownTank);
    alarm.handle(WateringEvent::UnknownTank);
    
    bool alarmStatus = alarm.isRaised();
    std::size_t counter = alarm.consecutiveAnomalies();
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(counter, 3u);

    alarm.acknowledge();
    alarmStatus = alarm.isRaised();
    counter = alarm.consecutiveAnomalies();
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(counter, 3u);
}

TEST_F(AlarmTest, SystemNeverWaterThePlant){
    std::unique_ptr<FakeStrategy> fakeStrategyOne = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);

    Alarm<3> alarm;
    std::unique_ptr<IWateringStrategy> emptyPointer;
    WateringController controller = createController(std::move(emptyPointer), minWaterLevel_);
    subscribeAlarm(controller, alarm);
    
    //First decision
    WateringDecision decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Wait);
    bool alarmStatus = alarm.isRaised();
    EXPECT_EQ(alarm.consecutiveAnomalies(), 1u);
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(callCounter_, 0);

    //Second decision
    decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Wait);
    alarmStatus = alarm.isRaised();
    EXPECT_EQ(alarm.consecutiveAnomalies(), 2u);
    EXPECT_FALSE(alarmStatus);
    EXPECT_EQ(callCounter_, 0);

    //Third decision
    decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Wait);
    alarmStatus = alarm.isRaised();
    EXPECT_EQ(alarm.consecutiveAnomalies(), 3u);
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(callCounter_, 0);

    //Change the strategy for the fake one
    controller.changeStrategy(std::move(fakeStrategyOne));
    
    //Alarm should still be raised, counter down to zero
    decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Water);
    alarmStatus = alarm.isRaised();
    EXPECT_EQ(alarm.consecutiveAnomalies(), 0u);
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(callCounter_, 1);
    
    //Alarm should still be raised, counter down to zero
    decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Water);
    alarmStatus = alarm.isRaised();
    EXPECT_EQ(alarm.consecutiveAnomalies(), 0u);
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(callCounter_, 2);

    //acknwoledge
    alarm.acknowledge();
    alarmStatus = alarm.isRaised();
    EXPECT_EQ(alarm.consecutiveAnomalies(), 0u);
    EXPECT_FALSE(alarmStatus);

}


TEST_F(AlarmTest, TwoIndependentSubscribers){
    std::unique_ptr<FakeStrategy> fakeStrategyOne = std::make_unique<FakeStrategy>(WateringDecision::Water, callCounter_);

    Alarm<3> alarm;
    EventJournal<10> journal;
    std::unique_ptr<IWateringStrategy> emptyPointer;
    WateringController controller = createController(std::move(emptyPointer), minWaterLevel_);
    subscribeAlarm(controller, alarm);
    subscribeJournal(controller, journal);
    
    //3 decisions
    WateringDecision decision = controller.decide(context_);
    decision = controller.decide(context_);
    decision = controller.decide(context_);
    
    EXPECT_EQ(decision, WateringDecision::Wait);
    
    //Alarm
    bool alarmStatus = alarm.isRaised();
    EXPECT_EQ(alarm.consecutiveAnomalies(), 3u);
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(callCounter_, 0);

    //Journal
    EXPECT_EQ(journal.at(0), WateringEvent::StrategyMissing);
    EXPECT_EQ(journal.at(1), WateringEvent::StrategyMissing);
    EXPECT_EQ(journal.at(2), WateringEvent::StrategyMissing);

    controller.changeStrategy(std::move(fakeStrategyOne));
    decision = controller.decide(context_);

    EXPECT_EQ(decision, WateringDecision::Water);

    //Alarm
    alarmStatus = alarm.isRaised();
    EXPECT_EQ(alarm.consecutiveAnomalies(), 0u);
    EXPECT_TRUE(alarmStatus);
    EXPECT_EQ(callCounter_, 1);

    //Journal
    EXPECT_EQ(journal.at(0), WateringEvent::StrategyMissing);
    EXPECT_EQ(journal.at(1), WateringEvent::StrategyMissing);
    EXPECT_EQ(journal.at(2), WateringEvent::StrategyMissing);
    EXPECT_EQ(journal.at(3), WateringEvent::StrategyDecisionWater);

}