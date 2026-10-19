#include <QtTest>
#include <QAbstractButton>
#include <QAction>
#include <QRadioButton>
#include <QPointer>
#include "widgets/CalculatorWindow.h"
#include "widgets/CalculatorKey.h"
using namespace Win7Calc;
using C=CalculationManager::Command;
static void enter(CalculatorController&c,QString text){for(auto ch:text){if(ch.isDigit())c.send(static_cast<C>(130+ch.digitValue()));else if(ch=='.')c.send(C::CommandPNT);}}
static void click(CalculatorWindow&w,const char*id){auto*b=w.findChild<QAbstractButton*>(id);QVERIFY2(b,id);QTest::mouseClick(b,Qt::LeftButton);}
class ScientificTest:public QObject {
 Q_OBJECT
private slots:
 void initTestCase(){QApplication::setFont(QFont("Segoe UI",9));}
 void precedence(){CalculatorController c;c.setMode(Mode::Scientific);enter(c,"2");c.send(C::CommandADD);enter(c,"3");c.send(C::CommandMUL);enter(c,"4");c.send(C::CommandEQU);QCOMPARE(c.state().display.primary,L"14");}
 void originalModeActionAndGrid(){CalculatorController c;CalculatorWindow w(c);auto*a=w.findChild<QAction*>("mode-1");QVERIFY(a);QVERIFY(a->isEnabled());a->trigger();QCOMPARE(w.size(),QSize(407,284));QCOMPARE(keyLayout(Mode::Scientific).size(),std::size_t(53));auto*blank=w.findChild<QAbstractButton*>("scientific-blank");QVERIFY(blank);QVERIFY(!blank->isEnabled());auto*sine=w.findChild<QAbstractButton*>("sin");QVERIFY(sine);QCOMPARE(sine->geometry(),QRect(89,149,34,27));}
 void angleUnits(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Scientific);w.show();enter(c,"30");click(w,"sin");QCOMPARE(c.state().display.primary,L"0.5");c.send(C::CommandCLEAR);click(w,"angle-radians");click(w,"pi");c.send(C::CommandDIV);enter(c,"2");c.send(C::CommandEQU);click(w,"sin");QCOMPARE(c.state().display.primary,L"1");c.send(C::CommandCLEAR);click(w,"angle-grads");enter(c,"100");click(w,"sin");QCOMPARE(c.state().display.primary,L"1");}
 void inverseLabelsAndLifetime(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Scientific);w.show();QPointer<QAbstractButton> inv=w.findChild<QAbstractButton*>("inverse");QVERIFY(inv);click(w,"inverse");QVERIFY(inv);QVERIFY(c.state().inverse);QVERIFY(inv->isChecked());QCOMPARE(w.findChild<QAbstractButton*>("ln")->text(),QString("eˣ"));QCOMPARE(w.findChild<QAbstractButton*>("int")->text(),QString("Frac"));QCOMPARE(w.findChild<QAbstractButton*>("sin")->text(),QString("sin⁻¹"));enter(c,"0.5");click(w,"sin");QCOMPARE(c.state().display.primary,L"30");QVERIFY(!c.state().inverse);QVERIFY(inv);QVERIFY(!inv->isChecked());}
 void inverseParity(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Scientific);w.show();enter(c,"12.3");click(w,"inverse");click(w,"dms");QCOMPARE(c.state().display.primary,L"12.5");c.send(C::CommandCLEAR);enter(c,"12.5");click(w,"inverse");click(w,"int");QCOMPARE(c.state().display.primary,L"0.5");c.send(C::CommandCLEAR);enter(c,"0");click(w,"inverse");click(w,"sinh");QCOMPARE(c.state().display.primary,L"0");}
 void inversePersistsForUnaffectedFunction(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Scientific);w.show();enter(c,"5");click(w,"inverse");click(w,"factorial");QCOMPARE(c.state().display.primary,L"120");QVERIFY(c.state().inverse);click(w,"clear");QVERIFY(!c.state().inverse);}
 void domainRecovery(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Scientific);w.show();enter(c,"1");c.send(C::CommandSIGN);click(w,"ln");QVERIFY(c.state().display.error);QCOMPARE(c.state().display.primary,L"Invalid input");click(w,"clear");enter(c,"8");c.send(C::CommandSIGN);click(w,"cube-root");QCOMPARE(c.state().display.primary,L"-2");}
 void exponentAndPowers(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Scientific);w.show();enter(c,"1");click(w,"exponent");enter(c,"3");click(w,"equals");QCOMPARE(c.state().display.primary,L"1000");click(w,"clear");enter(c,"2");click(w,"power");enter(c,"3");click(w,"equals");QCOMPARE(c.state().display.primary,L"8");click(w,"clear");enter(c,"27");click(w,"root");enter(c,"3");click(w,"equals");QCOMPARE(c.state().display.primary,L"3");}
 void scientificFormat(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Scientific);w.show();enter(c,"123.45");click(w,"scientific-format");QCOMPARE(c.state().display.primary,L"1.2345e+2");click(w,"scientific-format");QCOMPARE(c.state().display.primary,L"123.45");}
 void captionsFit(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Scientific);for(bool inverse:{false,true}){if(inverse)c.send(C::CommandINV);for(auto*k:w.findChildren<CalculatorKey*>()){if(k->isEnabled())QVERIFY2(QFontMetrics(k->font()).horizontalAdvance(k->text())<=k->width()-4,qPrintable(k->text()));}for(auto*r:w.findChildren<QRadioButton*>())QVERIFY2(r->sizeHint().width()<=r->width(),qPrintable(r->text()));}}
 void captureScientific(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Scientific);w.show();QTest::qWait(100);QTest::mouseMove(&w,QPoint(2,25));QTest::qWait(20);auto path=qEnvironmentVariable("WIN7CALC_CAPTURE");if(!path.isEmpty()){QVERIFY(w.grab().save(path));c.send(C::CommandINV);path.replace(".png","-inverse.png");QVERIFY(w.grab().save(path));}}
 void repeatedModeTransitions(){CalculatorController c;CalculatorWindow w(c);w.show();for(int i=0;i<12;++i){c.setMode(Mode::Scientific);click(w,"angle-grads");click(w,"inverse");c.setMode(Mode::Standard);c.setMode(Mode::Scientific);QVERIFY(!c.state().inverse);auto*deg=w.findChild<QRadioButton*>("angle-degrees");QVERIFY(deg);QVERIFY(deg->isChecked());enter(c,"30");click(w,"sin");QCOMPARE(c.state().display.primary,L"0.5");c.setMode(Mode::Standard);}}
};
QTEST_MAIN(ScientificTest)
#include "scientific_test.moc"
