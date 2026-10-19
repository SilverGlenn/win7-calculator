// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include "engine/Engine.h"
namespace Win7Calc {
class CalculatorDisplay final:public QWidget {
 Q_OBJECT
public:
 explicit CalculatorDisplay(QWidget* parent=nullptr);
 void setState(const DisplayState&,qreal scale);
protected:
 void paintEvent(QPaintEvent*) override;
private:
 DisplayState state_;
 qreal scale_=1;
};
}
