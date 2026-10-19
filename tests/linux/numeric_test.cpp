#include <QtTest>
#include "engine/Engine.h"
#include "engine/Numeric.h"
using namespace Win7Calc;
static bool commaLocale=false;
class NumericTest : public QObject {
 Q_OBJECT
private slots:
 void standaloneParsing() {auto n=parseDecimal(L"0.1"),m=parseDecimal(L"0.2");QVERIFY(n.value&&m.value);QCOMPARE(formatDecimal(*n.value+*m.value),L"0.3");}
 void exactDecimal() {Engine init;auto a=parseDecimal(L"0.1"),b=parseDecimal(L"0.2"),c=parseDecimal(L"0.3");QVERIFY(a.value&&b.value&&c.value);QVERIFY(*a.value+*b.value==*c.value);QCOMPARE(formatDecimal(*c.value),L"0.3");}
 void rejectMalformed() {Engine init;for(auto text:{L"",L"12foo",L"1+2",L"NaN",L"1e999999",L"1..2",L"1,2,3"})QVERIFY(!parseDecimal(text).value);QVERIFY(!parseDecimal(std::wstring(4097,L'9')).value);}
 void exponentAndSign() {Engine init;auto a=parseDecimal(L"\u22121.25e2");QVERIFY(a.value);QCOMPARE(formatDecimal(*a.value),L"-125");auto b=parseDecimal(L"2.5e-2");QVERIFY(b.value);QCOMPARE(formatDecimal(*b.value),L"0.025");}
 void localeIsolation() {if(!commaLocale)QSKIP("comma locale exercised in separate process");NumericLocale l{L',',L'.',L"3;0"};Engine e(l);auto a=parseDecimal(L"1,5",l);QVERIFY(a.value);QCOMPARE(formatDecimal(*a.value,l),L"1,5");QVERIFY(e.pasteNumber(L"1,5"));QCOMPARE(e.state().primary,L"1,5");}
};
int main(int argc,char**argv){QCoreApplication app(argc,argv);QStringList args=app.arguments();commaLocale=args.removeAll("--comma")>0;NumericTest test;return QTest::qExec(&test,args);}
#include "numeric_test.moc"
