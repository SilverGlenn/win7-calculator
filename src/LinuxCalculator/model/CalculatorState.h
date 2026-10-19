// SPDX-License-Identifier: MIT
#pragma once
#include "engine/Engine.h"
namespace Win7Calc {
enum class Panel { Basic, Units, Dates, Mortgage, VehicleLease, FuelMpg, FuelMetric };
enum class AngleUnit { Degrees, Radians, Grads };
struct CalculatorState {
 Mode mode = Mode::Standard;
 Panel panel = Panel::Basic;
 AngleUnit angle = AngleUnit::Degrees;
 // Original View checkbox preference persists while unavailable in other modes.
 bool historyVisible = false, digitGrouping = false, inverse = false;
 DisplayState display;
};
}
