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
 int radix=10, wordBits=64;
 std::uint64_t bitPattern=0;
 std::vector<std::wstring> samples;
 int selectedSample=-1;
 // Original View checkbox preference persists while unavailable in other modes.
 bool historyVisible = false, digitGrouping = false, inverse = false;
 DisplayState display;
};
}
