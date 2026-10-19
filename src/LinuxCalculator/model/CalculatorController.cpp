// SPDX-License-Identifier: MIT
#include "CalculatorController.h"
#include <QApplication>
#include <QClipboard>
#include <QLocale>
#include <QRegularExpression>
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
void CalculatorController::syncSamples() {
    state_.samples.clear();
    for(const auto&value:samples_)state_.samples.push_back(formatDecimal(value,currentNumericLocale(),16));
}
void CalculatorController::addSample() {
    if(state_.mode!=Mode::Statistics||engine_.state().error)return;
    statisticNotice_.clear();samples_.push_back(engine_.value());syncSamples();
    state_.selectedSample=static_cast<int>(samples_.size())-1;
    engine_.send(Command::CommandCENTR);refresh();
}
void CalculatorController::clearSamples() {
    samples_.clear();syncSamples();state_.selectedSample=-1;statisticNotice_.clear();
    engine_.send(Command::CommandCLEAR);refresh();
}
void CalculatorController::selectSample(int index) {
    if(index<0||index>=static_cast<int>(samples_.size()))return;
    state_.selectedSample=index;refresh();
}
void CalculatorController::updateSample(int index,const QString& text) {
    if(index<0||index>=static_cast<int>(samples_.size()))return;
    const auto parsed=parseDecimal(text.toStdWString(),currentNumericLocale());
    if(!parsed.value){emit invalidInput(tr("Invalid input"));refresh();return;}
    samples_[index]=*parsed.value;syncSamples();refresh();
}
void CalculatorController::computeStatistic(Statistic statistic) {
    if(state_.mode!=Mode::Statistics||engine_.state().error)return;
    const auto result=calculateStatistic(samples_,statistic);statisticNotice_.clear();
    if(result.value)engine_.setValue(*result.value);
    else statisticNotice_=result.error==NumericError::InsufficientData?L"Enter data to calculate":result.error==NumericError::Overflow?L"Overflow":L"Invalid input";
    refresh();
}
void CalculatorController::copySamples() {
    const auto index=state_.selectedSample;
    if(index>=0&&index<static_cast<int>(state_.samples.size()))
        QApplication::clipboard()->setText(QString::fromStdWString(state_.samples[index]));
}
void CalculatorController::pasteSamples() {
    const auto text=QApplication::clipboard()->text();
    if(text.isEmpty()||text.size()>4096){emit invalidInput(tr("Invalid input"));return;}
    std::vector<CalcEngine::Rational> pending;
    for(const auto&line:text.split(QRegularExpression("[\\r\\n]+"),Qt::SkipEmptyParts)){
        const auto parsed=parseDecimal(line.toStdWString(),currentNumericLocale());
        if(!parsed.value){emit invalidInput(tr("Invalid input"));return;}
        pending.push_back(*parsed.value);
    }
    if(pending.empty()){emit invalidInput(tr("Invalid input"));return;}
    samples_.insert(samples_.end(),pending.begin(),pending.end());syncSamples();
    state_.selectedSample=static_cast<int>(samples_.size())-1;statisticNotice_.clear();
    engine_.send(Command::CommandCENTR);refresh();
}
bool CalculatorController::historyAvailable() const {
    return state_.mode == Mode::Standard || state_.mode == Mode::Scientific;
}
void CalculatorController::refresh() {
    state_.display = engine_.state();
    if (!historyAvailable()) state_.display.history.clear();
    if(state_.mode==Mode::Statistics){
        state_.display.expression.clear();
        if(!statisticNotice_.empty()){state_.display.primary=statisticNotice_;state_.display.notice=true;}
    }
    if (state_.mode == Mode::Programmer && !state_.display.error) {
        const auto mask = state_.wordBits == 64 ? UINT64_MAX : ((std::uint64_t{1} << state_.wordBits) - 1);
        state_.bitPattern = engine_.value().ToUInt64_t() & mask;
    }
    emit stateChanged();
}
void CalculatorController::setMode(Mode mode) {
    if (state_.mode == mode) return;
    state_.mode = mode;
    state_.inverse = false;
    statisticNotice_.clear();
    if(mode==Mode::Statistics)state_.selectedSample=static_cast<int>(samples_.size())-1;
    engine_.setMode(mode == Mode::Statistics ? Mode::Standard : mode);
    state_.angle = AngleUnit::Degrees;
    if (mode == Mode::Scientific) engine_.send(Command::CommandDEG);
    if (mode == Mode::Programmer) state_.radix = 10;
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
    statisticNotice_.clear();
    engine_.send(command);
    if (!engine_.state().error) switch(command) {
    case Command::CommandHex: state_.radix=16; break;
    case Command::CommandDec: state_.radix=10; break;
    case Command::CommandOct: state_.radix=8; break;
    case Command::CommandBin: state_.radix=2; break;
    case Command::CommandQword: state_.wordBits=64; break;
    case Command::CommandDword: state_.wordBits=32; break;
    case Command::CommandWord: state_.wordBits=16; break;
    case Command::CommandByte: state_.wordBits=8; break;
    default: break;
    }
    if (command == Command::CommandINV) state_.inverse = !state_.inverse;
    if (command == Command::CommandDEG) state_.angle = AngleUnit::Degrees;
    if (command == Command::CommandRAD) state_.angle = AngleUnit::Radians;
    if (command == Command::CommandGRAD) state_.angle = AngleUnit::Grads;
    if (command == Command::CommandCLEAR || command == Command::CommandCENTR) state_.inverse = false;
    if (!engine_.state().error) switch(command) {
    case Command::CommandCHOP: case Command::CommandSIN: case Command::CommandCOS: case Command::CommandTAN:
    case Command::CommandSINH: case Command::CommandCOSH: case Command::CommandTANH: case Command::CommandLN:
    case Command::CommandDMS: case Command::CommandDegrees: case Command::CommandPI:
        state_.inverse = false; break;
    default: break;
    }
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
