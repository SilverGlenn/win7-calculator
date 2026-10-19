// SPDX-License-Identifier: MIT
#include <QApplication>
#include <QFontDatabase>
#include "model/CalculatorController.h"
#include "widgets/CalculatorWindow.h"
int main(int argc,char**argv){
 QApplication app(argc,argv);
 app.setApplicationName("win7-calculator");app.setApplicationDisplayName("Calculator");
 app.setOrganizationName("Win7Calculator");app.setDesktopFileName("org.win7calculator.Calculator");
 auto font=app.font();if(QFontDatabase::families().contains("Segoe UI")){font.setFamily("Segoe UI");app.setFont(font);}
 Win7Calc::CalculatorController controller;Win7Calc::CalculatorWindow window(controller);window.show();return app.exec();
}
