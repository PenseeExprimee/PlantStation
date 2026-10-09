#include <gtest/gtest.h>
#include <type_traits>
#include <optional>
#include <limits>
#include "plant/MovingAverage.hpp"

using namespace plant;

TEST(MovingAverage, ShouldReturnOptionalDouble)
{
    MovingAverage<int,1> movingAverage;
    EXPECT_TRUE((std::is_same_v< decltype(movingAverage.average()), std::optional<double>>));
}

TEST(MovingAverage, AverageOnEmptyFilterReturnsNullopt)
{
    MovingAverage<int, 5> movingAverage;
    EXPECT_EQ(movingAverage.average(), std::nullopt);
}

TEST(MovingAverage, OnlyOneValueAverageIsThisValue)
{
    MovingAverage<int, 3> movingAverage;
    movingAverage.add(10);
    EXPECT_EQ(movingAverage.average(), 10);
}


TEST(MovingAverage, MoreValueThanTheWindowReturnsAverageNLastValues)
{
    MovingAverage<int, 4> movingAverage;
    movingAverage.add(20);
    movingAverage.add(20);
    movingAverage.add(20);
    movingAverage.add(20);
    movingAverage.add(15);
    movingAverage.add(10);
    movingAverage.add(10);
    movingAverage.add(1);
    EXPECT_EQ(movingAverage.average(), 9);
}

TEST(MovingAverage, NonIntegerAverage)
{
    MovingAverage<int, 2> movingAverage;
    movingAverage.add(2);
    movingAverage.add(3);
    auto value = movingAverage.average();
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(*value, 2.5);
}

TEST(MovingAverage, TemplateWithDouble)
{
    MovingAverage<double, 2> movingAverage;
    movingAverage.add(2.5);
    movingAverage.add(4.5);
    auto value = movingAverage.average();
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(*value, 3.5);
}

TEST(MovingAverage, NonFullBuffer)
{
    MovingAverage<int, 5> movingAverage;
    movingAverage.add(2);
    movingAverage.add(4);
    
    EXPECT_EQ(movingAverage.average(), 3);
}

TEST(MovingAverage, FullBuffer)
{
    MovingAverage<int, 5> movingAverage;
    movingAverage.add(1);
    movingAverage.add(2);
    movingAverage.add(3);
    movingAverage.add(4);
    movingAverage.add(10);
    
    EXPECT_EQ(movingAverage.average(), 4);
}

TEST(MovingAverage, NegativeValuesOk)
{
    MovingAverage<int, 5> movingAverage;
    movingAverage.add(-4);
    movingAverage.add(2);
    
    EXPECT_EQ(movingAverage.average(), -1);
}

TEST(MovingAverage, LargeValuesDoNotOverflow)
{
    MovingAverage<int, 5> movingAverage;
    movingAverage.add(std::numeric_limits<int>::max());
    movingAverage.add(std::numeric_limits<int>::max());
    auto value = movingAverage.average();
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(*value, std::numeric_limits<int>::max());
}