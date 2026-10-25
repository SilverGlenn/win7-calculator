#include <QtTest>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "engine/Engine.h"
#include "engine/Resources.h"
#include "Header Files/EngineStrings.h"
using namespace Win7Calc;
using Command = CalculationManager::Command;
static void input(Engine& e, const QString& text) {
  for (auto ch : text) {
    if(ch.isDigit()) e.send(static_cast<Command>(130 + ch.digitValue()));
    else {
      const QMap<QChar,Command> keys{{'+',Command::CommandADD},{'-',Command::CommandSUB},{'*',Command::CommandMUL},
        {'/',Command::CommandDIV},{'.',Command::CommandPNT},{'%',Command::CommandPERCENT},{'=',Command::CommandEQU},
        {'C',Command::CommandCLEAR},{'E',Command::CommandCENTR},{'B',Command::CommandBACK}};
      QVERIFY2(keys.contains(ch),qPrintable(text)); e.send(keys[ch]);
    }
  }
}
class EngineTest : public QObject {
 Q_OBJECT
private slots:
 void standardParity() {
   QFile f(QStringLiteral(FIXTURE_DIR "/vm-cases.json")); QVERIFY(f.open(QIODevice::ReadOnly));
   const auto cases=QJsonDocument::fromJson(f.readAll()).object()["standard"].toArray(); QVERIFY(cases.size()>=9);
   for (const auto& v:cases) {const auto c=v.toObject(); Engine e; input(e,c["keys"].toString());
     QCOMPARE(QString::fromStdWString(e.state().primary),c["result"].toString());}
 }
 void singleLegacyMemory() {
   Engine e; input(e,"12"); e.send(Command::CommandSTORE); QVERIFY(e.state().memory);
   input(e,"C3");e.send(Command::CommandMPLUS);input(e,"C");e.send(Command::CommandRECALL);
   QCOMPARE(e.state().primary,L"15");input(e,"C2");e.send(Command::CommandMMINUS);
   input(e,"C");e.send(Command::CommandRECALL);QCOMPARE(e.state().primary,L"13");
   e.send(Command::CommandMCLEAR);QVERIFY(!e.state().memory); input(e,"C");e.send(Command::CommandRECALL);QCOMPARE(e.state().primary,L"0");
 }
 void errorRecovery() {Engine e;input(e,"1/0=");QVERIFY(e.state().error);QCOMPARE(e.state().primary,L"Cannot divide by zero");input(e,"C9");e.send(Command::CommandSQRT);QVERIFY(!e.state().error);QCOMPARE(e.state().primary,L"3");}
 void invalidPasteKeepsCalculation() {Engine e;input(e,"12+");QVERIFY(!e.pasteNumber(L"garbage"));input(e,"3=");QCOMPARE(e.state().primary,L"15");QVERIFY(!e.pasteNumber(std::wstring(4097,L'9')));}
 void numericPaste() {Engine e; QVERIFY(e.pasteNumber(L"\u221212.5")); QCOMPARE(e.state().primary,L"-12.5"); e.send(Command::CommandADD);QVERIFY(e.pasteNumber(L"2.5"));e.send(Command::CommandEQU);QCOMPARE(e.state().primary,L"-10");}
 void programmerPasteUsesActiveRadix_data() {
   QTest::addColumn<int>("radix"); QTest::addColumn<QString>("text"); QTest::addColumn<QString>("want");
   QTest::newRow("hex-letters") << int(Command::CommandHex) << "FF" << "FF";
   QTest::newRow("hex-digits") << int(Command::CommandHex) << "10" << "10";
   QTest::newRow("hex-E-is-digit") << int(Command::CommandHex) << "1e2" << "1E2";
   QTest::newRow("hex-unsigned-qword") << int(Command::CommandHex) << "FFFFFFFFFFFFFFFF" << "FFFFFFFFFFFFFFFF";
   QTest::newRow("octal") << int(Command::CommandOct) << "377" << "377";
   QTest::newRow("binary") << int(Command::CommandBin) << "10101010" << "10101010";
   QTest::newRow("decimal-min") << int(Command::CommandDec) << "-9223372036854775808" << "-9223372036854775808";
   QTest::newRow("unicode-minus") << int(Command::CommandHex) << QString::fromUtf8("  −a  ") << "FFFFFFFFFFFFFFF6";
 }
 void programmerPasteUsesActiveRadix() {
   QFETCH(int,radix); QFETCH(QString,text); QFETCH(QString,want);
   Engine e; e.setMode(Mode::Programmer); e.send(static_cast<Command>(radix));
   QVERIFY(e.pasteNumber(text.toStdWString()));
   QCOMPARE(QString::fromStdWString(e.state().primary),want);
 }
 void programmerInvalidPasteIsAtomic() {
   const std::vector<std::pair<Command,QStringList>> cases{
     {Command::CommandHex,{"FG","1e+2","1.5","", "+",QString(4097,'F')}},
     {Command::CommandOct,{"78","1239"}}, {Command::CommandBin,{"102","12"}},
     {Command::CommandDec,{"1e2","1.5"}}};
   for (const auto& [radix,invalid]:cases) {
     Engine e; e.setMode(Mode::Programmer); e.send(radix); input(e,"11+");
     const auto primary=e.state().primary, expression=e.state().expression; const auto value=e.value();
     for (const auto& text:invalid) {
       QVERIFY2(!e.pasteNumber(text.toStdWString()),qPrintable(text));
       QCOMPARE(e.state().primary,primary); QCOMPARE(e.state().expression,expression); QVERIFY(e.value()==value);
     }
     input(e,"1="); QCOMPARE(e.state().primary,radix==Command::CommandBin?L"100":L"12");
   }
 }
 void pasteErrorRecoveryAndModeReset() {
   Engine e; e.setMode(Mode::Programmer); e.send(Command::CommandHex); input(e,"1/0=");
   QVERIFY(e.state().error); const auto error=e.state().primary;
   QVERIFY(!e.pasteNumber(L"FG")); QVERIFY(e.state().error); QCOMPARE(e.state().primary,error);
   QVERIFY(e.pasteNumber(L"E")); QVERIFY(!e.state().error); QCOMPARE(e.state().primary,L"E");
   e.setMode(Mode::Standard); QVERIFY(e.pasteNumber(L"1e2")); QCOMPARE(e.state().primary,L"100");
   e.setMode(Mode::Programmer); QVERIFY(e.pasteNumber(L"10")); QCOMPARE(e.state().primary,L"10");
   e.send(Command::CommandHex); QCOMPARE(e.state().primary,L"A");
 }
 void exactValueTransfer() {Engine e;auto third=parseDecimal(L"1");auto three=parseDecimal(L"3");QVERIFY(third.value&&three.value);auto value=*third.value / *three.value;e.setValue(value);QVERIFY(e.value()==value);e.send(Command::CommandMUL);input(e,"3=");QCOMPARE(e.state().primary,L"1");}
 void resourceCompleteness() { Resources r;for(auto id:g_sids) QVERIFY2(r.contains(id),qPrintable(QString::fromStdWString(std::wstring(id)))); QCOMPARE(r.GetCEngineString(L"sDecimal"),L".");QCOMPARE(r.GetCEngineString(L"sThousand"),L",");QCOMPARE(r.GetCEngineString(L"sGrouping"),L"3;0");}
};
QTEST_GUILESS_MAIN(EngineTest)
#include "engine_test.moc"
