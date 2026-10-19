// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include "model/CalculatorController.h"
#include "CalculatorKey.h"
namespace Win7Calc {
class Keypad final:public QWidget {
 Q_OBJECT
public:
 Keypad(CalculatorController&,QWidget* parent=nullptr);
 void sync(const CalculatorState&,qreal scale);
private:
 CalculatorController& controller_;
 std::vector<CalculatorKey*> keys_;
 Mode mode_=Mode::Statistics;
 bool inverse_=false;
 bool initialized_=false;
};
}
