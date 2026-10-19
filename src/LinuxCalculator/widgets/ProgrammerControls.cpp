// SPDX-License-Identifier: MIT
#include "ProgrammerControls.h"
#include "KeyLayout.h"
#include "LegacyPalette.h"
#include <QButtonGroup>
#include <QPainter>
#include <QRegion>
namespace Win7Calc {
namespace {
class BitButton final:public QAbstractButton {
public:
 explicit BitButton(QWidget*parent):QAbstractButton(parent){setFocusPolicy(Qt::StrongFocus);setMouseTracking(true);setAttribute(Qt::WA_Hover);}
protected:
 void paintEvent(QPaintEvent*) override {
  QPainter p(this);
  if(underMouse()&&isEnabled()){p.fillRect(rect(),QColor("#e7effb"));p.setPen(QColor("#7d9cbd"));p.drawRect(rect().adjusted(0,0,-1,-1));}
  p.setPen(LegacyPalette::keyText);p.setFont(font());p.drawText(rect(),Qt::AlignCenter,text());
 }
};
}
ProgrammerControls::ProgrammerControls(CalculatorController& c,QWidget*p):QWidget(p){
 setObjectName("programmer-controls");
 for(int bit=0;bit<64;++bit){
  auto*b=new BitButton(this);bits_[bit]=b;b->setObjectName(QString("bit-%1").arg(bit));b->setAccessibleName(tr("Bit %1").arg(bit));
  connect(b,&QAbstractButton::clicked,this,[&c,bit]{c.send(static_cast<CalculationManager::Command>(700+bit));});
 }
 auto*baseGroup=new QButtonGroup(this);auto*widthGroup=new QButtonGroup(this);
 const QStringList labels={"Hex","Dec","Oct","Bin"},ids={"radix-hex","radix-decimal","radix-octal","radix-binary"};
 const QStringList widthLabels={"Qword","Dword","Word","Byte"},widthIds={"width-qword","width-dword","width-word","width-byte"};
 for(int i=0;i<4;++i){
  bases_[i]=new QRadioButton(labels[i],this);bases_[i]->setObjectName(ids[i]);baseGroup->addButton(bases_[i],i);
  widths_[i]=new QRadioButton(widthLabels[i],this);widths_[i]->setObjectName(widthIds[i]);widthGroup->addButton(widths_[i],i);
 }
 connect(baseGroup,&QButtonGroup::idClicked,this,[&c](int i){c.send(static_cast<CalculationManager::Command>(313+i));});
 connect(widthGroup,&QButtonGroup::idClicked,this,[&c](int i){c.send(static_cast<CalculationManager::Command>(317+i));});
}
void ProgrammerControls::sync(const CalculatorState&s,qreal scale){
 setVisible(s.mode==Mode::Programmer);scale_=scale;wordBits_=s.wordBits;
 setGeometry(scaledRect(QRect(11,85,385,253),scale));
 QRegion mask(scaledRect(QRect(0,0,385,61),scale));mask+=scaledRect(QRect(0,66,73,91),scale);mask+=scaledRect(QRect(0,162,73,91),scale);setMask(mask);
 for(int bit=0;bit<64;++bit){
  const int row=bit>=32?0:1,offset=31-(bit%32),group=offset/4,column=offset%4;
  const int x=8+group*48+qFloor(column*7.5),w=qFloor((column+1)*7.5)-qFloor(column*7.5);
  bits_[bit]->setGeometry(scaledRect(QRect(x,3+26*row,w,14),scale));
  bits_[bit]->setText((s.bitPattern&(std::uint64_t{1}<<bit))?"1":"0");
  auto f=QFont("Consolas");f.setPixelSize(qRound(12*scale));bits_[bit]->setFont(f);
  bits_[bit]->setVisible(bit<s.wordBits);bits_[bit]->setEnabled(!s.display.error);
 }
 const int radices[]={16,10,8,2},widths[]={64,32,16,8};
 const auto radioStyle=QString("QRadioButton {spacing:%1px;color:#1e395b;} QRadioButton:disabled {color:#6d7b90;} QRadioButton::indicator {width:%2px;height:%2px;}").arg(qRound(3*scale)).arg(qRound(12*scale));
 for(int i=0;i<4;++i){
  bases_[i]->setGeometry(scaledRect(QRect(7,71+21*i,62,20),scale));widths_[i]->setGeometry(scaledRect(QRect(7,167+21*i,62,20),scale));
  auto f=font();f.setPixelSize(qRound(11*scale));
  for(auto*b:{bases_[i],widths_[i]}){b->setFont(f);if(b->styleSheet()!=radioStyle)b->setStyleSheet(radioStyle);b->setEnabled(!s.display.error);}
  bases_[i]->setChecked(s.radix==radices[i]);widths_[i]->setChecked(s.wordBits==widths[i]);
 }
 update();
}
void ProgrammerControls::paintEvent(QPaintEvent*){
 QPainter p(this);p.scale(scale_,scale_);p.setPen(LegacyPalette::displayBorder);p.setBrush(QColor("#d9e5f1"));
 p.drawRoundedRect(QRectF(.5,.5,384,60),2,2);p.drawRoundedRect(QRectF(.5,66.5,72,90),2,2);p.drawRoundedRect(QRectF(.5,162.5,72,90),2,2);
 QFont f("Consolas");f.setPixelSize(11);p.setFont(f);p.setPen(LegacyPalette::disabledText);
 auto index=[&](int n,int x,int y){p.drawText(QRect(x,y,20,14),Qt::AlignLeft|Qt::AlignVCenter,QString::number(n));};
 if(wordBits_==64){index(63,8,17);index(47,200,17);index(32,363,17);}
 if(wordBits_>=32)index(31,8,43);
 if(wordBits_>=16)index(15,200,43);
 index(0,370,43);
}
}
