// SPDX-License-Identifier: MIT
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Numeric.h"
#include "ExpressionCommandInterface.h"
namespace Win7Calc {
enum class Mode { Standard, Scientific, Programmer, Statistics };
struct HistoryEntry { std::wstring expression, result; std::vector<std::shared_ptr<IExpressionCommand>> commands; };
struct DisplayState { std::wstring primary = L"0", expression; bool error = false, memory = false, notice = false; std::vector<HistoryEntry> history; };
class Engine {
public:
  explicit Engine(NumericLocale locale = {});
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  void setMode(Mode);
  void send(CalculationManager::Command);
  bool pasteNumber(std::wstring_view);
  void setDigitGrouping(bool);
  const DisplayState& state() const;
  CalcEngine::Rational value() const;
  void setValue(const CalcEngine::Rational&);
  void selectHistory(std::size_t);
  void clearHistory();
  bool beginHistoryEdit(std::size_t);
  void cancelHistoryEdit();
private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
}
