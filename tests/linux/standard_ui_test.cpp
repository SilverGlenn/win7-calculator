#include <QtTest>
#include <QMenuBar>
#include <QMenu>
#include <QAbstractButton>
#include <QClipboard>
#include <QLineEdit>
#include <QFile>
#include <QTextBrowser>
#include "widgets/CalculatorWindow.h"
#include "widgets/CalculatorKey.h"
using namespace Win7Calc;
class StandardUiTest:public QObject {
 Q_OBJECT
private slots:
 void initTestCase(){QApplication::setFont(QFont("Segoe UI",9));}
 void originalMenusAndModern(){
  CalculatorController c;CalculatorWindow w(c);auto bar=w.findChild<QMenuBar*>("menu-bar");QVERIFY(bar);
  QStringList names;for(auto*a:bar->actions())names<<QString(a->text()).remove('&');QCOMPARE(names,QStringList({"View","Edit","Help","Modern"}));
  auto modern=bar->actions().last()->menu();QVERIFY(modern);QCOMPARE(modern->actions().size(),1);QCOMPARE(modern->actions()[0]->text(),"No modern features enabled");QVERIFY(!modern->actions()[0]->isEnabled());
  QCOMPARE(w.windowTitle(),"Calculator");QVERIFY(!(w.windowFlags()&Qt::FramelessWindowHint));QCOMPARE(w.size(),QSize(212,284));
 }
 void mouseAndKeyboard(){
  CalculatorController c;CalculatorWindow w(c);w.show();QTest::qWait(20);
  for(const auto&id:{"digit-1","digit-2","add","digit-3","equals"}){auto b=w.findChild<QAbstractButton*>(id);QVERIFY(b);QTest::mouseClick(b,Qt::LeftButton);}
  QCOMPARE(c.state().display.primary,L"15");
  QTest::keyClick(&w,Qt::Key_Escape);QTest::keyClicks(&w,"12+3");QTest::keyClick(&w,Qt::Key_Return);QCOMPARE(c.state().display.primary,L"15");
  auto zero=w.findChild<QAbstractButton*>("digit-0");QVERIFY(zero);QVERIFY(!zero->accessibleName().isEmpty());
 }
 void releaseOutsideDoesNotActivate(){CalculatorController c;CalculatorWindow w(c);w.show();auto b=w.findChild<QAbstractButton*>("digit-7");QVERIFY(b);QTest::mousePress(b,Qt::LeftButton);QTest::mouseMove(b,QPoint(-10,-10));QTest::mouseRelease(b,Qt::LeftButton,Qt::NoModifier,QPoint(-10,-10));QCOMPARE(c.state().display.primary,L"0");}
 void focusedTextFieldKeepsKeys(){CalculatorController c;CalculatorWindow w(c);QLineEdit input(&w);input.setGeometry(0,0,100,20);w.show();input.show();input.setFocus();QTest::keyClicks(&input,"123");QCOMPARE(input.text(),"123");QCOMPARE(c.state().display.primary,L"0");}
 void memoryAndGroupingActions(){CalculatorController c;CalculatorWindow w(c);auto group=w.findChild<QAction*>("digit-grouping");QVERIFY(group);QTest::keyClicks(&w,"12345");group->trigger();QCOMPARE(c.state().display.primary,L"12,345");}
 void largerFonts(){auto old=QApplication::font();QApplication::setFont(QFont("Segoe UI",12));{CalculatorController c;CalculatorWindow w(c);QVERIFY(w.width()>=282);auto bar=w.findChild<QMenuBar*>("menu-bar");QVERIFY(bar);QVERIFY(bar->actionGeometry(bar->actions().last()).right()<w.width());}QApplication::setFont(old);}
 void fallbackFontStillWorks(){auto old=QApplication::font();QApplication::setFont(QFont("Missing font for fallback verification",12));{CalculatorController c;CalculatorWindow w(c);w.show();for(auto*b:w.findChildren<CalculatorKey*>()){QVERIFY(!b->accessibleName().isEmpty());QVERIFY(b->width()>20);QVERIFY(b->height()>20);}auto seven=w.findChild<QAbstractButton*>("digit-7");QVERIFY(seven);QTest::mouseClick(seven,Qt::LeftButton);QCOMPARE(c.state().display.primary,L"7");}QApplication::setFont(old);}
 void embeddedHelpAndIcon(){CalculatorController c;CalculatorWindow w(c);QFile help(":/help.html");QVERIFY(help.open(QIODevice::ReadOnly));QVERIFY(help.readAll().contains("Esc"));QVERIFY(!QIcon(":/icons/calculator.svg").isNull());}
 void captureStandard(){CalculatorController c;CalculatorWindow w(c);w.show();QTest::qWait(100);const auto path=qEnvironmentVariable("WIN7CALC_CAPTURE");if(!path.isEmpty())QVERIFY(w.grab().save(path));}
};
QTEST_MAIN(StandardUiTest)
#include "standard_ui_test.moc"
