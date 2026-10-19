// SPDX-License-Identifier: MIT
#pragma once
#include <QObject>
#include <QKeyEvent>
#include "CalculatorState.h"
namespace Win7Calc {
class CalculatorController final : public QObject {
 Q_OBJECT
public:
 explicit CalculatorController(QObject* parent = nullptr);
 const CalculatorState& state() const { return state_; }
 bool historyAvailable() const;
 bool handleKey(QKeyEvent*);
 void setMode(Mode);
 void setPanel(Panel);
 void setHistoryVisible(bool);
 void setDigitGrouping(bool);
 void send(CalculationManager::Command);
 void copy();
 void paste();
 void selectHistory(int);
 void beginHistoryEdit(int);
 void cancelHistoryEdit();
 void clearHistory();
 void copyHistory();
private:
 void refresh();
 Engine engine_;
 CalculatorState state_;
signals:
 void stateChanged();
 void invalidInput(const QString&);
};
}
