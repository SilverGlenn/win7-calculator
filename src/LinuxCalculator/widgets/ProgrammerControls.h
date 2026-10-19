// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include <QAbstractButton>
#include <QRadioButton>
#include "model/CalculatorController.h"
namespace Win7Calc {
class ProgrammerControls final:public QWidget {
 Q_OBJECT
public:
 ProgrammerControls(CalculatorController&,QWidget* parent=nullptr);
 void sync(const CalculatorState&,qreal scale);
protected:
 void paintEvent(QPaintEvent*) override;
private:
 QAbstractButton* bits_[64];
 QRadioButton* bases_[4];
 QRadioButton* widths_[4];
 qreal scale_=1;
 int wordBits_=64;
};
}
