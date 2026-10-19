// SPDX-License-Identifier: MIT
#pragma once
#include "Numeric.h"
#include "CalculatorResource.h"
namespace Win7Calc {
class Resources final : public CalculationManager::IResourceProvider {
public:
  explicit Resources(NumericLocale locale = {}) : locale_(locale) {}
  std::wstring GetCEngineString(std::wstring_view id) override;
  bool contains(std::wstring_view id) const;
private: NumericLocale locale_;
};
}
