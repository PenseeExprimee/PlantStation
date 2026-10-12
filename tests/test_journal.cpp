#include <gtest/gtest.h>
#include <optional>
#include <memory>
#include <utility>
#include <cstddef>
#include "plant/WateringEnum.hpp"
#include "plant/EventJournal.hpp"
#include "plant/IWateringStrategy.hpp"
#include "plant/WateringController.hpp"
#include "utils/test_helpers.hpp"

using namespace plant;


class JournalTest : public ::testing::Test
{
protected:
    double minWaterLevel_ = 20.0;
    WateringContext context_ = createContext(40, 100);
    int callCounter_ = 0;
};

TEST(JournalEvents, ANewJournalIsEmpty){
    EventJournal<30> eventJournal;

    std::size_t journalSize = eventJournal.size();
    std::optional<WateringEvent> event = eventJournal.at(0); 
    EXPECT_EQ(journalSize, 0u);
    EXPECT_EQ(event, std::nullopt);
}

TEST(JournalEvents, CanReadAnEventInIndex0){
    EventJournal<30> eventJournal;
    eventJournal.save(WateringEvent::LowWaterLevel);

    std::optional<WateringEvent> event = eventJournal.at(0); 
    EXPECT_EQ(event, WateringEvent::LowWaterLevel);
}

TEST(JournalEvents, KeepTheNLastValuesInOrder){
    EventJournal<3> eventJournal;
    
    eventJournal.save(WateringEvent::LowWaterLevel);
    eventJournal.save(WateringEvent::UnknownTank);
    eventJournal.save(WateringEvent::StrategyDecisionWait);
    eventJournal.save(WateringEvent::StrategyDecisionWater);

    std::optional<WateringEvent> eventZero = eventJournal.at(0); 
    std::optional<WateringEvent> eventOne = eventJournal.at(1); 
    std::optional<WateringEvent> eventTwo = eventJournal.at(2); 
    std::optional<WateringEvent> eventThree = eventJournal.at(3); 
    std::size_t journalSize = eventJournal.size();

    EXPECT_EQ(eventZero, WateringEvent::UnknownTank);
    EXPECT_EQ(eventOne, WateringEvent::StrategyDecisionWait);
    EXPECT_EQ(eventTwo, WateringEvent::StrategyDecisionWater);
    EXPECT_EQ(eventThree, std::nullopt);
    EXPECT_EQ(journalSize, 3u);
}


TEST_F(JournalTest, theJournalContainsTheDecisionsSequenceAndForgetsTheOldest){
    EventJournal<3> eventJournal;
    std::unique_ptr<FakeStrategy> fakeStrategy = std::make_unique<FakeStrategy>(WateringDecision::Wait, callCounter_);
    WateringController controller = createController(std::move(fakeStrategy), minWaterLevel_);
    subscribeJournal(controller, eventJournal);

    //Normal context: Consult the strategy that says wait
    WateringDecision decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Wait);
    EXPECT_EQ(eventJournal.at(0), WateringEvent::StrategyDecisionWait);
    EXPECT_EQ(callCounter_, 1);

    //Low water level context
    context_.waterLevel = 2.0;
    decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Wait);
    EXPECT_EQ(eventJournal.at(1), WateringEvent::LowWaterLevel);
    EXPECT_EQ(callCounter_, 1);

    context_.waterLevel = std::nullopt;
    decision = controller.decide(context_);
    EXPECT_EQ(decision, WateringDecision::Wait);
    EXPECT_EQ(eventJournal.at(2), WateringEvent::UnknownTank);
    EXPECT_EQ(callCounter_, 1);

    std::optional<WateringEvent> eventZero = eventJournal.at(0); 
    std::optional<WateringEvent> eventOne = eventJournal.at(1); 
    std::optional<WateringEvent> eventTwo = eventJournal.at(2); 
    std::optional<WateringEvent> eventThree = eventJournal.at(3); 
    std::size_t journalSize = eventJournal.size();

    EXPECT_EQ(eventZero, WateringEvent::StrategyDecisionWait);
    EXPECT_EQ(eventOne, WateringEvent::LowWaterLevel);
    EXPECT_EQ(eventTwo, WateringEvent::UnknownTank);
    EXPECT_EQ(eventThree, std::nullopt);
    EXPECT_EQ(journalSize, 3u);

    //original values of the context
    context_.waterLevel = 20;
    decision = controller.decide(context_);

    eventZero = eventJournal.at(0); 
    eventOne = eventJournal.at(1); 
    eventTwo = eventJournal.at(2); 
    eventThree = eventJournal.at(3); 
    EXPECT_EQ(decision, WateringDecision::Wait);

    EXPECT_EQ(eventZero, WateringEvent::LowWaterLevel);
    EXPECT_EQ(eventOne, WateringEvent::UnknownTank);
    EXPECT_EQ(eventTwo, WateringEvent::StrategyDecisionWait);
    EXPECT_EQ(eventThree, std::nullopt);
    EXPECT_EQ(callCounter_, 2);

}