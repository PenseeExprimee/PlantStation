#include <gtest/gtest.h>
#include <iostream>
#include "plant/EventText.hpp"
#include "plant/WateringEnum.hpp"

using namespace plant;

TEST(EventText, EventLowWater){
    
    std::string_view event = eventText(WateringEvent::LowWaterLevel);
    EXPECT_EQ(event, "Not enough water left in the tank.");
}

TEST(EventText, EventStrategyDecisionWait){
    std::string_view event = eventText(WateringEvent::StrategyDecisionWait);
    EXPECT_EQ(event, "No watering needed. Let's wait.");
}

TEST(EventText, EventStrategyDecisionWater){
    std::string_view  event = eventText(WateringEvent::StrategyDecisionWater);
    EXPECT_EQ(event, "The plant needs watering.");
}

TEST(EventText, EventUnknownTank){
    std::string_view event = eventText(WateringEvent::UnknownTank);
    EXPECT_EQ(event, "The level of the tank cannot be read.");
}

TEST(EventText, EventStrategyIsMissing){
    std::string_view event = eventText(WateringEvent::StrategyMissing);
    EXPECT_EQ(event, "The strategy is missing.");
   
}

TEST(EventText, EventUnknownEvent){
    std::string_view event = eventText(static_cast<WateringEvent>(99));
    EXPECT_EQ(event, "Unknown event.");
   
}