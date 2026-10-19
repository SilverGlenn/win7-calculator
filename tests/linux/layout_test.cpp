#include <QtTest>
#include "widgets/KeyLayout.h"
using namespace Win7Calc;
class LayoutTest:public QObject {
 Q_OBJECT
private slots:
 void referenceGeometry() {
  QCOMPARE(clientSize(Mode::Standard,Panel::Basic,false,1),QSize(212,284));
  QCOMPARE(clientSize(Mode::Scientific,Panel::Basic,false,1),QSize(407,284));
  QCOMPARE(clientSize(Mode::Programmer,Panel::Basic,true,1),QSize(407,350));
  QCOMPARE(clientSize(Mode::Statistics,Panel::Basic,false,1),QSize(212,384));
  QCOMPARE(clientSize(Mode::Standard,Panel::Units,true,1),QSize(586,384));
  auto keys=keyLayout(Mode::Standard,false);QCOMPARE(keys.size(),std::size_t(28));
  auto find=[&](QString id){for(auto&k:keys)if(k.id==id)return k.rect;return QRect();};
  QCOMPARE(find("digit-7"),QRect(11,149,33,27));QCOMPARE(find("digit-0"),QRect(11,245,73,27));
  QCOMPARE(find("equals"),QRect(167,213,34,59));QCOMPARE(find("memory-clear"),QRect(11,85,33,27));
  QCOMPARE(find("memory-recall"),QRect(50,85,34,27));QCOMPARE(find("memory-store"),QRect(89,85,34,27));
 }
 void scalesFit() {for(auto s:{1.0,1.25,1.5,2.0}){auto size=clientSize(Mode::Standard,Panel::Basic,false,s);QVERIFY(size.width()>=qRound(212*s));for(auto&k:keyLayout(Mode::Standard,false))QVERIFY(QRect(QPoint(),size).contains(scaledRect(k.rect,s)));}}
};
QTEST_GUILESS_MAIN(LayoutTest)
#include "layout_test.moc"
