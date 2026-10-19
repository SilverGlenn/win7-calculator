// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include <QListWidget>
#include <QAbstractButton>
#include "model/CalculatorController.h"
namespace Win7Calc {
class StatisticsControls final:public QWidget {
 Q_OBJECT
public:
 StatisticsControls(CalculatorController&,QWidget* parent=nullptr);
 void sync(const CalculatorState&,qreal);
protected:
 void paintEvent(QPaintEvent*) override;
private:
 CalculatorController& controller_;
 QListWidget* samples_;
 QAbstractButton *up_,*down_;
 qreal scale_=1;
 int count_=0;
};
}
