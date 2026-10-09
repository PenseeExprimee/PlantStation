#include <gtest/gtest.h>
#include "plant/RingBuffer.hpp"

using namespace plant;

TEST(RingBuffer, NewBufferIsEmptyNotFull)
{
    RingBuffer<int, 60> ringBuffer;

    EXPECT_TRUE(ringBuffer.empty());
    EXPECT_FALSE(ringBuffer.full());
}

TEST(RingBuffer, AfterTwoAddContains2Elements)
{
    RingBuffer<int, 3> ringBuffer;
    ringBuffer.add(41);
    ringBuffer.add(42);

    EXPECT_EQ(ringBuffer.size(), 2u);
}

TEST(RingBuffer, AfterThreeAddIsFull)
{
    RingBuffer<int, 3> ringBuffer;
    ringBuffer.add(40);
    ringBuffer.add(41);
    ringBuffer.add(42);

    EXPECT_FALSE(ringBuffer.empty());
    EXPECT_TRUE(ringBuffer.full());
}

TEST(RingBuffer, CounterAfterFullNessMustBeN)
{
    RingBuffer<int, 3> ringBuffer;
    ringBuffer.add(40);
    ringBuffer.add(41);
    ringBuffer.add(42);
    ringBuffer.add(17);

    EXPECT_TRUE(ringBuffer.full());
    EXPECT_EQ(ringBuffer.size(), 3u);
}

TEST(RingBuffer, ChronologicalContentAfterFull)
{
    RingBuffer<int, 3> ringBuffer;
    ringBuffer.add(40);
    ringBuffer.add(41);
    ringBuffer.add(42);
    ringBuffer.add(17);

    EXPECT_EQ(ringBuffer.at(0), 41);
    EXPECT_EQ(ringBuffer.at(1), 42);
    EXPECT_EQ(ringBuffer.at(2), 17);
}

TEST(RingBuffer, TestTemplateWithDouble)
{
    RingBuffer<double, 3> ringBuffer;
    ringBuffer.add(3.3);
    ringBuffer.add(2.6);
    ringBuffer.add(7.2);
    ringBuffer.add(4.8);

    auto value = ringBuffer.at(0);
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(*value, 2.6);
}

TEST(RingBuffer, PartiallyFullBuffer)
{
    RingBuffer<double, 5> ringBuffer;
    ringBuffer.add(3.3);
    ringBuffer.add(2.6);

    auto value = ringBuffer.at(0);
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(*value, 3.3);
    EXPECT_FALSE(ringBuffer.at(8).has_value());
}

TEST(RingBuffer, EmptyBuffer)
{
    RingBuffer<double, 5> ringBuffer;

    EXPECT_EQ(ringBuffer.at(0), std::nullopt);
    EXPECT_EQ(ringBuffer.at(3), std::nullopt);
    EXPECT_EQ(ringBuffer.at(7), std::nullopt);
}

TEST(RingBuffer, LastValidIndex)
{
    RingBuffer<double, 5> ringBuffer;

    ringBuffer.add(40);
    ringBuffer.add(41);
    ringBuffer.add(42);
    auto value = ringBuffer.at(2);
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(*value, 42);
}

TEST(RingBuffer, LastValidIndexFullBuffer)
{
    RingBuffer<double, 5> ringBuffer;

    ringBuffer.add(40);
    ringBuffer.add(41);
    ringBuffer.add(42);
    ringBuffer.add(82);
    ringBuffer.add(92);
    ringBuffer.add(21);
    auto value = ringBuffer.at(4);
    ASSERT_TRUE(value.has_value());
    EXPECT_DOUBLE_EQ(*value, 21);
}

TEST(RingBuffer, FirstInvalidIndex)
{
    RingBuffer<double, 5> ringBuffer;

    ringBuffer.add(40);
    ringBuffer.add(41);
    ringBuffer.add(42);
    ringBuffer.add(19);
    ringBuffer.add(20);
    EXPECT_EQ(ringBuffer.at(5), std::nullopt);
}

TEST(RingBuffer, FirstInvalidIndexPartiallyFull)
{
    RingBuffer<double, 5> ringBuffer;

    ringBuffer.add(40);
    ringBuffer.add(41);
    EXPECT_EQ(ringBuffer.at(2), std::nullopt);
}

TEST(RingBuffer, BufferStateAfterManyLoops)
{
    RingBuffer<double, 5> ringBuffer;

    for (int i = 1; i <= 22; i++)
    {
        ringBuffer.add(i);
    }
    auto valueAtFour = ringBuffer.at(4);
    ASSERT_TRUE(valueAtFour.has_value());
    EXPECT_DOUBLE_EQ(*valueAtFour, 22);

    auto valueAtZero = ringBuffer.at(0);
    ASSERT_TRUE(valueAtZero.has_value());
    EXPECT_DOUBLE_EQ(*valueAtZero, 18);
}