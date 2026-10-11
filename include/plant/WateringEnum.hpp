#pragma once
enum class WateringEvent
{
    StrategyMissing,
    UnknownTank,
    LowWaterLevel,
    StrategyDecisionWait,
    StrategyDecisionWater
};

enum class WateringDecision
{
    Water,
    Wait
};