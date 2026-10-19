// SPDX-License-Identifier: MIT
#include "CalculatorController.h"
#include <QApplication>
#include <QClipboard>
#include <QLocale>
#include <optional>

namespace Win7Calc {
using Command = CalculationManager::Command;
namespace {
NumericLocale currentNumericLocale() {
    const QLocale locale;
    const auto decimal = locale.decimalPoint().toStdWString();
    const auto grouping = locale.groupSeparator().toStdWString();
    return {decimal.empty() ? L'.' : decimal.front(), grouping.empty() ? L',' : grouping.front(), L"3;0"};
}
}
CalculatorController::CalculatorController(QObject* parent)
    : QObject(parent), engine_(currentNumericLocale()) {
    state_.display = engine_.state();
}
bool CalculatorController::historyAvailable() const {
    return state_.mode == Mode::Standard || state_.mode == Mode::Scientific;
}
void CalculatorController::refresh() {
    state_.display = engine_.state();
    if (!historyAvailable()) state_.display.history.clear();
    emit stateChanged();
}
void CalculatorController::setMode(Mode mode) {
    if (state_.mode == mode) return;
    state_.mode = mode;
    state_.inverse = false;
    engine_.setMode(mode == Mode::Statistics ? Mode::Standard : mode);
    refresh();
}
void CalculatorController::setPanel(Panel panel) {
    if (state_.panel == panel) return;
    state_.panel = panel;
    refresh();
}
void CalculatorController::setHistoryVisible(bool visible) {
    if (!historyAvailable()) return;
    state_.historyVisible = visible;
    refresh();
}
void CalculatorController::setDigitGrouping(bool grouping) {
    state_.digitGrouping = grouping;
    engine_.setDigitGrouping(grouping);
    refresh();
}
void CalculatorController::send(Command command) {
    engine_.send(command);
    refresh();
}
void CalculatorController::copy() {
    QApplication::clipboard()->setText(QString::fromStdWString(state_.display.primary));
}
void CalculatorController::paste() {
    const auto text = QApplication::clipboard()->text();
    if (!engine_.pasteNumber(text.toStdWString())) {
        emit invalidInput(tr("Invalid input"));
        return;
    }
    refresh();
}
bool CalculatorController::handleKey(QKeyEvent* event) {
    if (event->type() != QEvent::KeyPress ||
        (event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) return false;
    std::optional<Command> command;
    const auto key = event->key();
    if (key >= Qt::Key_0 && key <= Qt::Key_9) command = static_cast<Command>(130 + key - Qt::Key_0);
    else if (state_.mode == Mode::Programmer && key >= Qt::Key_A && key <= Qt::Key_F)
        command = static_cast<Command>(140 + key - Qt::Key_A);
    else switch (key) {
    case Qt::Key_Plus: command = Command::CommandADD; break;
    case Qt::Key_Minus: command = Command::CommandSUB; break;
    case Qt::Key_Asterisk: command = Command::CommandMUL; break;
    case Qt::Key_Slash: command = Command::CommandDIV; break;
    case Qt::Key_Period: case Qt::Key_Comma: command = Command::CommandPNT; break;
    case Qt::Key_Return: case Qt::Key_Enter: case Qt::Key_Equal: command = Command::CommandEQU; break;
    case Qt::Key_Backspace: command = Command::CommandBACK; break;
    case Qt::Key_Delete: command = Command::CommandCENTR; break;
    case Qt::Key_Escape: command = Command::CommandCLEAR; break;
    case Qt::Key_Percent: command = Command::CommandPERCENT; break;
    case Qt::Key_F9: command = Command::CommandSIGN; break;
    case Qt::Key_ParenLeft: command = Command::CommandOPENP; break;
    case Qt::Key_ParenRight: command = Command::CommandCLOSEP; break;
    default: break;
    }
    if (!command) return false;
    send(*command);
    event->accept();
    return true;
}
}
