// SPDX-License-Identifier: MIT
#include "ScientificControls.h"
#include "KeyLayout.h"
#include "LegacyPalette.h"
#include <QButtonGroup>
#include <QPainter>
namespace Win7Calc {
ScientificControls::ScientificControls(CalculatorController& c,QWidget* p):QWidget(p){
 setObjectName("scientific-angle-controls");auto*group=new QButtonGroup(this);
 const QStringList labels={tr("Degrees"),tr("Radians"),tr("Grads")};
 const QStringList ids={"angle-degrees","angle-radians","angle-grads"};
 for(int i=0;i<3;++i){angles_[i]=new QRadioButton(labels[i],this);angles_[i]->setObjectName(ids[i]);group->addButton(angles_[i],i);}
 connect(group,&QButtonGroup::idClicked,this,[&c](int id){c.send(static_cast<CalculationManager::Command>(321+id));});
}
void ScientificControls::sync(const CalculatorState& s,qreal scale){
 setVisible(s.mode==Mode::Scientific);
 setGeometry(scaledRect(QRect(11,85+(s.historyVisible?100:0),190,27),scale));
 const QRect positions[]={QRect(8,3,62,21),QRect(72,3,67,21),QRect(139,3,49,21)};
 const auto radioStyle=QString("QRadioButton { spacing:%1px; color:#1e395b; } QRadioButton::indicator { width:%2px; height:%2px; }").arg(qRound(3*scale)).arg(qRound(12*scale));
 for(int i=0;i<3;++i){angles_[i]->setGeometry(scaledRect(positions[i],scale));auto f=font();f.setPixelSize(qRound(11*scale));angles_[i]->setFont(f);if(angles_[i]->styleSheet()!=radioStyle)angles_[i]->setStyleSheet(radioStyle);angles_[i]->setChecked(i==static_cast<int>(s.angle));}
}
void ScientificControls::paintEvent(QPaintEvent*){
 QPainter p(this);p.setPen(LegacyPalette::displayBorder);p.setBrush(QColor("#d9e5f1"));p.drawRoundedRect(QRectF(.5,.5,width()-1,height()-1),2,2);
}
}
