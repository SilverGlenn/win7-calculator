// SPDX-License-Identifier: MIT
#pragma once
#include <span>
#include "engine/Numeric.h"
namespace Win7Calc {
enum class Statistic { Mean, MeanSquare, Sum, SumSquare, PopulationDeviation, SampleDeviation };
NumericResult calculateStatistic(std::span<const CalcEngine::Rational>,Statistic);
}
