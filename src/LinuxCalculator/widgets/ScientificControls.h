// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include <QRadioButton>
#include "model/CalculatorController.h"
namespace Win7Calc {
class ScientificControls final:public QWidget {
 Q_OBJECT
public:
 ScientificControls(CalculatorController&,QWidget* parent=nullptr);
 void sync(const CalculatorState&,qreal scale);
protected:
 void paintEvent(QPaintEvent*) override;
private:
 QRadioButton* angles_[3];
};
}
