#include <QtTest>
#include <QAbstractButton>
#include <QRadioButton>
#include <QAction>
#include "widgets/CalculatorWindow.h"
using namespace Win7Calc;
using C=CalculationManager::Command;
static void enter(CalculatorController&c,QString s){for(auto ch:s)c.send(static_cast<C>(ch.isDigit()?130+ch.digitValue():140+ch.toUpper().unicode()-'A'));}
static void click(CalculatorWindow&w,const char*id){auto*b=w.findChild<QAbstractButton*>(id);QVERIFY2(b,id);QVERIFY2(b->isEnabled(),id);QTest::mouseClick(b,Qt::LeftButton);}
class ProgrammerTest:public QObject {
 Q_OBJECT
private slots:
 void initTestCase(){QApplication::setFont(QFont("Segoe UI",9));}
 void radixAndDisabledKeys(){CalculatorController c;CalculatorWindow w(c);auto*mode=w.findChild<QAction*>("mode-2");QVERIFY(mode);QVERIFY(mode->isEnabled());mode->trigger();w.show();QCOMPARE(w.size(),QSize(407,350));QCOMPARE(keyLayout(Mode::Programmer).size(),std::size_t(46));for(auto id:{"hex-A","decimal","percent","square-root","reciprocal"}){auto*b=w.findChild<QAbstractButton*>(id);QVERIFY(b);QVERIFY(!b->isEnabled());}click(w,"radix-hex");click(w,"hex-A");QCOMPARE(c.state().display.primary,L"A");click(w,"radix-binary");QCOMPARE(c.state().display.primary,L"1010");QVERIFY(!w.findChild<QAbstractButton*>("digit-2")->isEnabled());}
 void signedWidths(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Programmer);w.show();const C widths[]={C::CommandByte,C::CommandWord,C::CommandDword,C::CommandQword};const QStringList hex={"FF","FFFF","FFFFFFFF","FFFFFFFFFFFFFFFF"};for(int i=0;i<4;++i){c.send(widths[i]);c.send(C::CommandCLEAR);click(w,"bit-not");QCOMPARE(c.state().display.primary,L"-1");click(w,"radix-hex");QCOMPARE(QString::fromStdWString(c.state().display.primary),hex[i]);click(w,"radix-decimal");QCOMPARE(c.state().display.primary,L"-1");}}
 void highestBitToggle(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Programmer);w.show();const C cmds[]={C::CommandByte,C::CommandWord,C::CommandDword,C::CommandQword};const int widths[]={8,16,32,64};const QStringList expected={"-128","-32768","-2147483648","-9223372036854775808"};for(int i=0;i<4;++i){c.send(cmds[i]);c.send(C::CommandCLEAR);auto id=QString("bit-%1").arg(widths[i]-1).toLatin1();click(w,id);QCOMPARE(QString::fromStdWString(c.state().display.primary),expected[i]);QCOMPARE(c.state().bitPattern,std::uint64_t{1}<<(widths[i]-1));}c.send(C::CommandByte);QVERIFY(w.findChild<QAbstractButton*>("bit-63")->isHidden());}
 void narrowing(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Programmer);w.show();click(w,"radix-hex");enter(c,"1234");c.send(C::CommandByte);QCOMPARE(c.state().display.primary,L"34");c.send(C::CommandCLEAR);enter(c,"80");c.send(C::CommandDword);QCOMPARE(c.state().display.primary,L"FFFFFF80");}
 void shiftRotateParity(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Programmer);c.send(C::CommandByte);w.show();enter(c,"1");click(w,"rotate-left");QCOMPARE(c.state().display.primary,L"2");c.send(C::CommandCLEAR);enter(c,"1");click(w,"rotate-right");QCOMPARE(c.state().display.primary,L"-128");c.send(C::CommandCLEAR);click(w,"bit-7");click(w,"shift-right");enter(c,"1");click(w,"equals");QCOMPARE(c.state().display.primary,L"-64");c.send(C::CommandCLEAR);enter(c,"1");click(w,"shift-left");enter(c,"8");click(w,"equals");QVERIFY(c.state().display.error);QCOMPARE(c.state().display.primary,L"Result not defined");QCOMPARE(c.state().bitPattern,std::uint64_t{8});QVERIFY(!w.findChild<QAbstractButton*>("digit-1")->isEnabled());QVERIFY(w.findChild<QAbstractButton*>("clear")->isEnabled());}
 void modeSwitchState(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Programmer);c.send(C::CommandByte);c.send(C::CommandHex);enter(c,"80");c.setMode(Mode::Standard);c.setMode(Mode::Programmer);QCOMPARE(c.state().wordBits,8);QCOMPARE(c.state().radix,10);QCOMPARE(c.state().display.primary,L"0");auto*byte=w.findChild<QRadioButton*>("width-byte");QVERIFY(byte);QVERIFY(byte->isChecked());}
 void nativeHitTesting(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Programmer);w.show();QVERIFY(QTest::qWaitForWindowExposed(&w));auto*seven=w.findChild<QAbstractButton*>("digit-7");QVERIFY(seven);QTest::mouseClick(w.windowHandle(),Qt::LeftButton,Qt::NoModifier,seven->mapTo(&w,seven->rect().center()));QCOMPARE(c.state().display.primary,L"7");}
 void captureProgrammer(){CalculatorController c;CalculatorWindow w(c);c.setMode(Mode::Programmer);w.show();c.send(C::CommandBINPOS63);QTest::qWait(100);QTest::mouseMove(&w,QPoint(2,25));auto path=qEnvironmentVariable("WIN7CALC_CAPTURE");if(!path.isEmpty())QVERIFY(w.grab().save(path));}
};
QTEST_MAIN(ProgrammerTest)
#include "programmer_test.moc"
