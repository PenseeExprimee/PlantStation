#include <gtest/gtest.h>

#include <memory>

#include "plant/FakeSensor.hpp"
#include "plant/ISensor.hpp"

using namespace plant;

TEST(FakeSensor, ReturnsScriptedValuesInOrder) {
    FakeSensor sensor("humidity", {42.0, 55.5});

    EXPECT_EQ(sensor.read(), 42.0);
    EXPECT_EQ(sensor.read(), 55.5);
}

TEST(FakeSensor, ReturnsNulloptWhenScriptIsExhausted) {
    FakeSensor sensor("humidity", {10.0});

    sensor.read();
    EXPECT_FALSE(sensor.read().has_value());
}

TEST(FakeSensor, CanSimulateInvalidReading) {
    FakeSensor sensor("humidity", {std::nullopt, 30.0});

    EXPECT_FALSE(sensor.read().has_value());
    EXPECT_EQ(sensor.read(), 30.0);
}

TEST(ISensor, WorksThroughPolymorphicPointer) {
    std::unique_ptr<ISensor> sensor =
        std::make_unique<FakeSensor>("level", std::vector<std::optional<double>>{80.0});

    EXPECT_EQ(sensor->name(), "level");
    EXPECT_EQ(sensor->read(), 80.0);
}
