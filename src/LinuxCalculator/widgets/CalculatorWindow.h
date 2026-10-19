// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include <QMenuBar>
#include "model/CalculatorController.h"
#include "CalculatorDisplay.h"
#include "Keypad.h"
namespace Win7Calc {
class CalculatorWindow final:public QWidget {
 Q_OBJECT
public:
 explicit CalculatorWindow(CalculatorController&,QWidget* parent=nullptr);
protected:
 void paintEvent(QPaintEvent*) override;
 void keyPressEvent(QKeyEvent*) override;
private:
 void buildMenus();
 void refresh();
 void showHelp();
 CalculatorController& controller_;
 QMenuBar* menus_;
 CalculatorDisplay* display_;
 Keypad* keypad_;
 qreal scale_=1;
};
}
