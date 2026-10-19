// SPDX-License-Identifier: MIT
#include "CalculatorDisplay.h"
#include "LegacyPalette.h"
#include <QPainter>
#include <QLinearGradient>
namespace Win7Calc {
CalculatorDisplay::CalculatorDisplay(QWidget* p):QWidget(p){setObjectName("display");setAccessibleName(tr("Calculator display"));}
void CalculatorDisplay::setState(const DisplayState& state,qreal scale){state_=state;scale_=scale;setAccessibleDescription(QString::fromStdWString(state.primary));update();}
void CalculatorDisplay::paintEvent(QPaintEvent*) {
 QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
 QLinearGradient fill(0,2,0,height()-3);fill.setColorAt(0,QColor("#edf3fc"));fill.setColorAt(1,Qt::white);
 p.setPen(LegacyPalette::displayBorder);p.setBrush(fill);p.drawRoundedRect(QRectF(.5,.5,width()-1,height()-1),2,2);
 p.setPen(Qt::white);p.setBrush(Qt::NoBrush);p.drawRoundedRect(QRectF(1.5,1.5,width()-3,height()-3),1,1);
 QFont f("Consolas");f.setPixelSize(qRound(23*scale_));p.setFont(f);
 QString primary=QString::fromStdWString(state_.primary);
 const QRect resultRect(qRound(5*scale_),height()-qRound(32*scale_),width()-qRound(10*scale_),qRound(29*scale_));
 while(f.pixelSize()>qRound(11*scale_)&&QFontMetrics(f).horizontalAdvance(primary)>resultRect.width()){
  f.setPixelSize(f.pixelSize()-1);
 }
 p.setFont(f);p.setPen(Qt::black);p.drawText(resultRect,Qt::AlignRight|Qt::AlignVCenter,primary);
 if(!state_.expression.empty()){
  f.setPixelSize(qRound(11*scale_));p.setFont(f);
  QString expr=QString::fromStdWString(state_.expression);
  expr=QFontMetrics(f).elidedText(expr,Qt::ElideLeft,width()-qRound(10*scale_));
  p.setPen(LegacyPalette::keyText);p.drawText(QRect(qRound(5*scale_),qRound(4*scale_),width()-qRound(10*scale_),qRound(14*scale_)),Qt::AlignRight|Qt::AlignVCenter,expr);
 }
 if(state_.memory){f=font();f.setPixelSize(qRound(11*scale_));p.setFont(f);p.setPen(LegacyPalette::keyText);p.drawText(QRect(qRound(5*scale_),height()-qRound(22*scale_),qRound(16*scale_),qRound(16*scale_)),Qt::AlignLeft|Qt::AlignVCenter,"M");}
}
}
