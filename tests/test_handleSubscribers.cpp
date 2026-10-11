#include <gtest/gtest.h>
#include <vector>
#include <functional>
#include <string>
#include <utility>
#include "plant/SubscribeHandler.hpp"

using namespace plant;


TEST(SubscribeHandler, NoSubscriberNoProblem){
    SubscribeHandler subscriberHandler;
    subscriberHandler.diffuse(WateringEvent::LowWaterLevel);
    SUCCEED();
}

TEST(SubscribeHandler, SubscriberIsCalled){
    SubscribeHandler subscriberHandler;
    int counter = 0;
    std::function<void(WateringEvent)> sayHello = [&counter](WateringEvent){
        counter++;
    };
    subscriberHandler.subscribe(std::move(sayHello));
    subscriberHandler.diffuse(WateringEvent::LowWaterLevel);
    EXPECT_EQ(counter, 1);
}

TEST(SubscribeHandler, SubscribersGetTheSameEventInOrder){
    SubscribeHandler subscriberHandler;
    std::vector<std::string> traceOriginal = {"Hello", "Bye"};
    std::vector<std::string> trace = {};
    auto sayHello = [&trace](WateringEvent event){
        if (event == WateringEvent::LowWaterLevel){
            trace.push_back("Hello");
        }
    };
    auto sayBye = [&trace](WateringEvent event){
        if (event == WateringEvent::LowWaterLevel){
            trace.push_back("Bye");
        }
    };
    subscriberHandler.subscribe(std::move(sayHello));
    subscriberHandler.subscribe(std::move(sayBye));
    subscriberHandler.diffuse(WateringEvent::LowWaterLevel);
    EXPECT_EQ(trace, traceOriginal);
}

TEST(SubscribeHandler, LateSubscriberDoesNotGetPreviousUpdates){
    SubscribeHandler subscriberHandler;
    std::vector<std::string> expectedTrace = {"Hello"};
    std::vector<std::string> secondexpectedTrace = {"Hello","Hello", "Bye"};
    std::vector<std::string> trace = {};
    auto sayHello = [&trace](WateringEvent event){
        if (event == WateringEvent::LowWaterLevel){
            trace.push_back("Hello");
        }
    };
    subscriberHandler.subscribe(std::move(sayHello));
    subscriberHandler.diffuse(WateringEvent::LowWaterLevel);

    auto sayBye = [&trace](WateringEvent event){
        if (event == WateringEvent::LowWaterLevel){
            trace.push_back("Bye");
        }
    };
    subscriberHandler.subscribe(std::move(sayBye));
    EXPECT_EQ(trace, expectedTrace);
    subscriberHandler.diffuse(WateringEvent::LowWaterLevel);
    EXPECT_EQ(trace, secondexpectedTrace);
}

TEST(SubscribeHandler, SubscriberInternalStateKeepsTrack){
    SubscribeHandler subscriberHandler;
    int witness = 0;
    auto hasInternalState = [counter = 0, &witness](WateringEvent event) mutable {
        if (event == WateringEvent::LowWaterLevel){
            counter++;
            witness = counter;
        }
    };
    subscriberHandler.subscribe(std::move(hasInternalState));
    subscriberHandler.diffuse(WateringEvent::LowWaterLevel);
    subscriberHandler.diffuse(WateringEvent::StrategyMissing);
    subscriberHandler.diffuse(WateringEvent::LowWaterLevel);

    EXPECT_EQ(witness, 2);
}