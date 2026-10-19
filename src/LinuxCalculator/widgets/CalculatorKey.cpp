// SPDX-License-Identifier: MIT
#include "CalculatorKey.h"
#include "LegacyPalette.h"
#include <QPainter>
#include <QLinearGradient>
#include <QStyleOptionFocusRect>
namespace Win7Calc {
CalculatorKey::CalculatorKey(const KeySpec& spec,QWidget* parent):QAbstractButton(parent),spec_(spec) {
 setSpec(spec);
 setFocusPolicy(Qt::StrongFocus);setAttribute(Qt::WA_Hover);setMouseTracking(true);
}
void CalculatorKey::setSpec(const KeySpec& spec){
 spec_=spec;setObjectName(spec.id);setText(spec.text);setAccessibleName(spec.accessibleName);
 setEnabled(spec.enabled);setCheckable(spec.checkable);if(spec.checkable)setChecked(spec.checked);update();
}
void CalculatorKey::setLogicalScale(qreal s){
 scale_=s;
 QFont f=font();
 int pixels=spec_.fontPixels?spec_.fontPixels:(spec_.appearance==KeyAppearance::Number?14:(spec_.appearance==KeyAppearance::Memory?11:12));
 if(spec_.id=="equals")pixels=20;
 if(spec_.id=="backspace")pixels=21;
 f.setPixelSize(qRound(pixels*scale_));f.setBold(false);setFont(f);update();
}
void CalculatorKey::paintEvent(QPaintEvent*) {
 QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
 const QRectF bounds(.5,.5,width()-1,height()-1);
 const bool hot=isEnabled()&&underMouse(), pressed=isEnabled()&&(isDown()||isChecked());
 QLinearGradient fill(0,1,0,height()-2),border(0,0,0,height());
 if(pressed){
  border.setColorAt(0,QColor("#c28a30"));border.setColorAt(1,QColor("#c28a30"));
  fill.setColorAt(0,QColor("#fbd189"));fill.setColorAt(.49,QColor("#f5c779"));fill.setColorAt(.5,QColor("#f5b557"));fill.setColorAt(1,QColor("#fbe08a"));
 }else if(hot){
  border.setColorAt(0,QColor("#e5a100"));border.setColorAt(1,QColor("#e5a100"));
  fill.setColorAt(0,QColor("#fff4d8"));fill.setColorAt(.49,QColor("#ffe8b3"));fill.setColorAt(.5,QColor("#ffda77"));fill.setColorAt(1,QColor("#ffe59c"));
 }else{
  border.setColorAt(0,LegacyPalette::keyBorder);border.setColorAt(1,LegacyPalette::displayBorder);
  if(spec_.appearance==KeyAppearance::Number){
   fill.setColorAt(0,QColor("#f7fbff"));fill.setColorAt(.49,QColor("#edf3f8"));fill.setColorAt(.5,QColor("#e7edf5"));fill.setColorAt(1,QColor("#ebf3f9"));
  }else if(spec_.appearance==KeyAppearance::Memory){
   fill.setColorAt(0,QColor("#d4e0ed"));fill.setColorAt(1,QColor("#d7e2ef"));
  }else{
   fill.setColorAt(0,QColor("#f0f6fb"));fill.setColorAt(.49,QColor("#e2eaf3"));fill.setColorAt(.5,QColor("#d5e0ed"));fill.setColorAt(1,QColor("#d7e2ef"));
  }
 }
 p.setPen(QPen(QBrush(border),1));p.setBrush(fill);p.drawRoundedRect(bounds,2*scale_,2*scale_);
 p.setBrush(Qt::NoBrush);p.setPen(QColor(255,255,255,pressed?110:230));
 p.drawRoundedRect(bounds.adjusted(1,1,-1,-1),1*scale_,1*scale_);
 p.setFont(font());p.setPen(isEnabled()?LegacyPalette::keyText:LegacyPalette::disabledText);
 p.drawText(rect(),Qt::AlignCenter,text());
 if(hasFocus()&&style()->styleHint(QStyle::SH_UnderlineShortcut,nullptr,this)){
  QPen pen(QColor("#6d7584"));pen.setStyle(Qt::DotLine);p.setPen(pen);p.setBrush(Qt::NoBrush);
  p.drawRect(rect().adjusted(3,3,-4,-4));
 }
}
}
