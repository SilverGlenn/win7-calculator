// SPDX-License-Identifier: MIT
#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "Header Files/Rational.h"
namespace Win7Calc {
struct NumericLocale { wchar_t decimal = L'.'; wchar_t grouping = L','; std::wstring groupingPattern = L"3;0"; };
enum class NumericError { None, InvalidInput, Domain, DivideByZero, Overflow, InsufficientData };
struct NumericResult { std::optional<CalcEngine::Rational> value; NumericError error = NumericError::None; };
NumericResult parseDecimal(std::wstring_view text, NumericLocale locale = {});
std::wstring formatDecimal(const CalcEngine::Rational&, NumericLocale locale = {}, int significantDigits = 32);
}
