// SPDX-License-Identifier: MIT
#pragma once
#include <QObject>
#include <QKeyEvent>
#include "CalculatorState.h"
#include "legacy/Statistics.h"
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
 void addSample();
 void clearSamples();
 void selectSample(int);
 void updateSample(int,const QString&);
 void computeStatistic(Statistic);
private:
 void refresh();
 Engine engine_;
 CalculatorState state_;
 std::vector<CalcEngine::Rational> samples_;
 std::wstring statisticNotice_;
 void syncSamples();
signals:
 void stateChanged();
 void invalidInput(const QString&);
};
}
