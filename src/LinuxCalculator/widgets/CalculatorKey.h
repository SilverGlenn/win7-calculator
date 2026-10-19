// SPDX-License-Identifier: MIT
#pragma once
#include <QAbstractButton>
#include "KeyLayout.h"
namespace Win7Calc {
class CalculatorKey final:public QAbstractButton {
 Q_OBJECT
public:
 CalculatorKey(const KeySpec&,QWidget* parent=nullptr);
 void setLogicalScale(qreal);
protected:
 void paintEvent(QPaintEvent*) override;
private:
 KeySpec spec_;
 qreal scale_=1;
};
}
