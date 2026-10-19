// SPDX-License-Identifier: MIT
#include "Resources.h"
#include "EngineStringsData.h"
#include <algorithm>
namespace Win7Calc {
std::wstring Resources::GetCEngineString(std::wstring_view id) {
    if (id == L"sDecimal") return {locale_.decimal};
    if (id == L"sThousand") return {locale_.grouping};
    if (id == L"sGrouping") return locale_.groupingPattern;
    const auto it = std::find_if(std::begin(engineStringsData), std::end(engineStringsData),
                                [id](const auto& entry) { return entry.first == id; });
    return it == std::end(engineStringsData) ? std::wstring{} : std::wstring(it->second);
}
bool Resources::contains(std::wstring_view id) const {
    if (id == L"sDecimal" || id == L"sThousand" || id == L"sGrouping") return true;
    return std::any_of(std::begin(engineStringsData), std::end(engineStringsData),
                       [id](const auto& entry) { return entry.first == id; });
}
}
