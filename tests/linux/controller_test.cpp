#include <QtTest>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QLocale>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include "model/CalculatorController.h"
using namespace Win7Calc;
using Command=CalculationManager::Command;
static void type(CalculatorController& c,const QString& input) {
 for(auto ch:input){QKeyEvent e(QEvent::KeyPress,ch.isDigit()?Qt::Key_0+ch.digitValue():ch.unicode(),Qt::NoModifier,QString(ch));QVERIFY(c.handleKey(&e));}
}
class ControllerTest : public QObject {
 Q_OBJECT
private slots:
 void initialState(){CalculatorController c;QVERIFY(c.state().mode==Mode::Standard);QVERIFY(c.state().panel==Panel::Basic);QCOMPARE(c.state().display.primary,L"0");QVERIFY(!c.state().historyVisible);QVERIFY(!c.state().digitGrouping);}
 void transitionMatrix(){
  CalculatorController c;c.setHistoryVisible(true);type(c,"12");
  QFile f(QStringLiteral(FIXTURE_DIR "/ui-states.json"));QVERIFY(f.open(QIODevice::ReadOnly));
  for(auto v:QJsonDocument::fromJson(f.readAll()).object()["matrix"].toArray()){
   const auto row=v.toObject();c.setMode(static_cast<Mode>(row["modeIndex"].toInt()));c.setPanel(static_cast<Panel>(row["panelIndex"].toInt()));
   QCOMPARE(static_cast<int>(c.state().mode),row["modeIndex"].toInt());QCOMPARE(static_cast<int>(c.state().panel),row["panelIndex"].toInt());
   QVERIFY(c.state().historyVisible);QCOMPARE(c.historyAvailable(),row["historyAvailable"].toBool());
  }
  c.setMode(Mode::Standard);QCOMPARE(c.state().display.primary,L"0");QVERIFY(c.state().historyVisible);
 }
 void oneCommandPerKey(){CalculatorController c;type(c,"12+3");QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QVERIFY(c.handleKey(&enter));QCOMPARE(c.state().display.primary,L"15");QKeyEvent unmapped(QEvent::KeyPress,Qt::Key_F12,Qt::NoModifier);QVERIFY(!c.handleKey(&unmapped));QCOMPARE(c.state().display.primary,L"15");}
 void singleMemoryAcrossModes(){CalculatorController c;type(c,"12");c.send(Command::CommandSTORE);c.setMode(Mode::Scientific);c.send(Command::CommandRECALL);QCOMPARE(c.state().display.primary,L"12");c.setMode(Mode::Programmer);c.send(Command::CommandRECALL);QCOMPARE(c.state().display.primary,L"12");c.setMode(Mode::Standard);c.send(Command::CommandMCLEAR);QVERIFY(!c.state().display.memory);}
 void clipboardValidation(){CalculatorController c;type(c,"12+");QSignalSpy errors(&c,&CalculatorController::invalidInput);for(const auto&s:QStringList{"", "1+2", "NaN",QString(4097,'9')}){QApplication::clipboard()->setText(s);c.paste();}QCOMPARE(errors.count(),4);type(c,"3");c.send(Command::CommandEQU);QCOMPARE(c.state().display.primary,L"15");QApplication::clipboard()->setText(QString::fromUtf8("−12.5"));c.paste();QCOMPARE(c.state().display.primary,L"-12.5");c.copy();QCOMPARE(QApplication::clipboard()->text(),"-12.5");}
 void programmerErrorReentryClipboard_data(){
  QTest::addColumn<int>("radix");QTest::addColumn<bool>("leaveError");
  QTest::newRow("hex-error")<<int(Command::CommandHex)<<true;
  QTest::newRow("oct-error")<<int(Command::CommandOct)<<true;
  QTest::newRow("bin-error")<<int(Command::CommandBin)<<true;
  QTest::newRow("hex-normal")<<int(Command::CommandHex)<<false;
  QTest::newRow("oct-normal")<<int(Command::CommandOct)<<false;
  QTest::newRow("bin-normal")<<int(Command::CommandBin)<<false;
 }
 void programmerErrorReentryClipboard(){
  QFETCH(int,radix);QFETCH(bool,leaveError);
  CalculatorController c;c.setMode(Mode::Programmer);c.send(Command::CommandWord);
  c.send(static_cast<Command>(radix));type(c,leaveError?"1/0=":"11");
  QCOMPARE(c.state().display.error,leaveError);
  // Do not recover the old Programmer engine before switching away.
  c.setMode(Mode::Standard);c.setMode(Mode::Programmer);
  QVERIFY(!c.state().display.error);QCOMPARE(c.state().display.primary,L"0");
  QCOMPARE(c.state().radix,10);QCOMPARE(c.state().wordBits,16);
  QSignalSpy errors(&c,&CalculatorController::invalidInput);
  QApplication::clipboard()->setText("10");c.paste();
  QCOMPARE(errors.count(),0);QCOMPARE(c.state().radix,10);
  QCOMPARE(c.state().display.primary,L"10");QCOMPARE(c.state().bitPattern,std::uint64_t{10});
  c.copy();QCOMPARE(QApplication::clipboard()->text(),"10");
  c.send(Command::CommandCLEAR);c.paste();
  QCOMPARE(errors.count(),0);QCOMPARE(c.state().radix,10);QCOMPARE(c.state().wordBits,16);
  QCOMPARE(c.state().display.primary,L"10");QCOMPARE(c.state().bitPattern,std::uint64_t{10});
 }
 void groupingNotValueMutation(){CalculatorController c;type(c,"12345");c.setDigitGrouping(true);QCOMPARE(c.state().display.primary,L"12,345");c.setDigitGrouping(false);QCOMPARE(c.state().display.primary,L"12345");c.send(Command::CommandADD);type(c,"1");c.send(Command::CommandEQU);QCOMPARE(c.state().display.primary,L"12346");}
 void localizedClipboard(){QLocale old=QLocale();QLocale::setDefault(QLocale("de_DE"));{CalculatorController c;QApplication::clipboard()->setText("1,5");c.paste();QCOMPARE(c.state().display.primary,L"1,5");}QLocale::setDefault(old);}
};
QTEST_MAIN(ControllerTest)
#include "controller_test.moc"
