// SPDX-License-Identifier: MIT
#include "Numeric.h"
#include <algorithm>
#include <cwctype>
#include <limits>
namespace Win7Calc {
namespace {
void ensureNumericRuntime() {
    // The retained library owns process-global constants. All public operations
    // run on the owning thread, including helpers used without a GUI Engine.
    if (rat_one == nullptr) ChangeConstants(10, 32);
}
}
NumericResult parseDecimal(std::wstring_view text, NumericLocale locale) {
    auto invalid = [] { return NumericResult{std::nullopt, NumericError::InvalidInput}; };
    if (text.empty() || text.size() > 4096) return invalid();
    while (!text.empty() && std::iswspace(text.front())) text.remove_prefix(1);
    while (!text.empty() && std::iswspace(text.back())) text.remove_suffix(1);
    if (text.empty()) return invalid();
    bool negative = text.front() == L'-' || text.front() == L'\u2212';
    if (negative || text.front() == L'+') text.remove_prefix(1);
    std::wstring digits;
    bool fractional = false, grouped = false;
    int fractionDigits = 0, groupDigits = 0;
    std::size_t i = 0;
    for (; i < text.size() && text[i] != L'e' && text[i] != L'E'; ++i) {
        auto ch = text[i];
        if (ch >= L'0' && ch <= L'9') {
            digits.push_back(ch);
            if (fractional) ++fractionDigits; else ++groupDigits;
        } else if (ch == locale.decimal && !fractional) {
            if (grouped && groupDigits != 3) return invalid();
            fractional = true;
        } else if (ch == locale.grouping && !fractional) {
            if (groupDigits == 0 || (grouped ? groupDigits != 3 : groupDigits > 3)) return invalid();
            grouped = true; groupDigits = 0;
        } else return invalid();
    }
    if (digits.empty() || (grouped && !fractional && groupDigits != 3)) return invalid();
    int exponent = 0;
    if (i < text.size()) {
        ++i;
        bool exponentNegative = i < text.size() && text[i] == L'-';
        if (i < text.size() && (text[i] == L'-' || text[i] == L'+')) ++i;
        if (i == text.size()) return invalid();
        for (; i < text.size(); ++i) {
            if (text[i] < L'0' || text[i] > L'9') return invalid();
            exponent = exponent * 10 + static_cast<int>(text[i] - L'0');
            if (exponent > 9999) return invalid();
        }
        if (exponentNegative) exponent = -exponent;
    }
    exponent -= fractionDigits;
    if (exponent < -9999 || exponent > 9999) return invalid();
    try {
        ensureNumericRuntime();
        // Integer mantissa means this path never changes RatPack's global
        // decimal separator while another engine owns the active locale.
        PRAT raw = StringToRat(negative, digits, exponent < 0, std::to_wstring(std::abs(exponent)), 10, CalcEngine::RATIONAL_PRECISION);
        if (!raw) return invalid();
        CalcEngine::Rational value(raw);
        destroyrat(raw);
        return {std::move(value), NumericError::None};
    } catch (uint32_t) { return {std::nullopt, NumericError::Overflow}; }
}
std::wstring formatDecimal(const CalcEngine::Rational& value, NumericLocale locale, int significantDigits) {
    ensureNumericRuntime();
    auto text = value.ToString(10, NumberFormat::Float, significantDigits);
    for (auto& ch : text) if (ch == L'.' || ch == L',') ch = locale.decimal;
    return text;
}
}
