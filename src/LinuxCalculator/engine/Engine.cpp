// SPDX-License-Identifier: MIT
#include "Engine.h"
#include "Resources.h"
#include "CalculatorManager.h"
#include <algorithm>
#include <stdexcept>

namespace Win7Calc {
using Command = CalculationManager::Command;

class Engine::Impl final : public ICalcDisplay {
public:
    NumericLocale locale;
    Resources resources;
    DisplayState snapshot;
    std::unique_ptr<CalculationManager::CalculatorManager> manager;
    Mode mode = Mode::Standard;
    bool grouping = false;
    std::wstring rawPrimary;
    CalcEngine::Rational memory;

    explicit Impl(NumericLocale l) : locale(l), resources(l) {
        manager = std::make_unique<CalculationManager::CalculatorManager>(this, &resources);
        manager->SetStandardMode();
    }
    void refreshHistory() {
        if (mode == Mode::Programmer) { snapshot.history.clear(); return; }
        snapshot.history.clear();
        for (const auto& item : manager->GetHistoryItems()) {
            const auto& h = item->historyItemVector;
            snapshot.history.push_back({h.expression, h.result, h.spCommands ? *h.spCommands : std::vector<std::shared_ptr<IExpressionCommand>>{}});
        }
    }
    void refreshPrimary() {
        snapshot.primary = rawPrimary;
        if (!grouping && !snapshot.error) {
            std::erase(snapshot.primary, locale.grouping);
            std::erase(snapshot.primary, L' ');
        }
    }
    void SetPrimaryDisplay(const std::wstring& text, bool error) override {
        rawPrimary = text;
        snapshot.error = error;
        refreshPrimary();
    }
    void SetIsInError(bool error) override { snapshot.error = error; }
    void SetExpressionDisplay(std::shared_ptr<std::vector<std::pair<std::wstring,int>>> const& tokens,
                              std::shared_ptr<std::vector<std::shared_ptr<IExpressionCommand>>> const&) override {
        snapshot.expression.clear();
        if (tokens) for (const auto& token : *tokens) snapshot.expression += token.first;
    }
    void SetParenthesisNumber(unsigned int) override {}
    void OnNoRightParenAdded() override {}
    void MaxDigitsReached() override {}
    void BinaryOperatorReceived() override {}
    void OnHistoryItemAdded(unsigned int) override {}
    void SetMemorizedNumbers(const std::vector<std::wstring>&) override {}
    void MemoryItemChanged(unsigned int) override {}
    void InputChanged() override {}
};

Engine::Engine(NumericLocale locale) : impl_(std::make_unique<Impl>(locale)) {}
Engine::~Engine() = default;

void Engine::setMode(Mode mode) {
    if (mode == Mode::Statistics) return; // Statistics owns samples, not another evaluator.
    impl_->mode = mode;
    switch (mode) {
    case Mode::Standard: impl_->manager->SetStandardMode(); break;
    case Mode::Scientific: impl_->manager->SetScientificMode(); break;
    case Mode::Programmer: impl_->manager->SetProgrammerMode(); break;
    case Mode::Statistics: break;
    }
    impl_->manager->LegacyMemory(impl_->memory);
    impl_->refreshHistory();
}
void Engine::send(Command command) {
    impl_->manager->SendCommand(command);
    if (!impl_->snapshot.error && (command == Command::CommandSTORE || command == Command::CommandMCLEAR ||
        command == Command::CommandMPLUS || command == Command::CommandMMINUS)) {
        impl_->memory = impl_->manager->LegacyMemory();
        impl_->snapshot.memory = impl_->memory != CalcEngine::Rational(0);
    }
    impl_->refreshHistory();
}
bool Engine::pasteNumber(std::wstring_view text) {
    const auto parsed = parseDecimal(text, impl_->locale);
    if (!parsed.value) return false;
    setValue(*parsed.value);
    return !impl_->snapshot.error;
}
void Engine::setDigitGrouping(bool grouping) { impl_->grouping = grouping; impl_->refreshPrimary(); }
const DisplayState& Engine::state() const { return impl_->snapshot; }
CalcEngine::Rational Engine::value() const { return impl_->manager->CurrentValue(); }
void Engine::setValue(const CalcEngine::Rational& value) { impl_->manager->CurrentValue(value); impl_->refreshHistory(); }
void Engine::clearHistory() { impl_->manager->ClearHistory(); impl_->refreshHistory(); }
// Selection/editing are implemented with their UI and parity tests in Task 7.
}
