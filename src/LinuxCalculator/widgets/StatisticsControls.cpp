// SPDX-License-Identifier: MIT
#include "StatisticsControls.h"
#include "KeyLayout.h"
#include "LegacyPalette.h"
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QPainter>
#include <QSignalBlocker>
namespace Win7Calc {
namespace {
class NavigationButton final:public QAbstractButton {
 bool up_;
public:
 NavigationButton(bool up,QWidget*p):QAbstractButton(p),up_(up){setFocusPolicy(Qt::StrongFocus);setAccessibleName(up?tr("Previous data"):tr("Next data"));}
protected:
 void paintEvent(QPaintEvent*)override{
  QPainter p(this);p.setRenderHint(QPainter::Antialiasing);QLinearGradient fill(0,0,0,height());fill.setColorAt(0,Qt::white);fill.setColorAt(1,QColor("#e4effb"));
  p.setPen(QColor("#c2cfdd"));p.setBrush(fill);p.drawRoundedRect(QRectF(.5,.5,width()-1,height()-1),1,1);
  p.setPen(Qt::NoPen);p.setBrush(isEnabled()?QColor("#335574"):QColor("#adbfcf"));
  const qreal cx=width()/2.0,cy=height()/2.0,dx=width()/4.0,dy=height()/7.0;
  QPolygonF arrow;if(up_)arrow<<QPointF(cx,cy-dy)<<QPointF(cx-dx,cy+dy)<<QPointF(cx+dx,cy+dy);
  else arrow<<QPointF(cx,cy+dy)<<QPointF(cx-dx,cy-dy)<<QPointF(cx+dx,cy-dy);
  p.drawPolygon(arrow);
 }
};
}
StatisticsControls::StatisticsControls(CalculatorController&c,QWidget*p):QWidget(p),controller_(c),samples_(new QListWidget(this)),up_(new NavigationButton(true,this)),down_(new NavigationButton(false,this)){
 setObjectName("statistics-controls");samples_->setObjectName("statistics-samples");samples_->setFrameShape(QFrame::NoFrame);
 samples_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);samples_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 samples_->setSelectionMode(QAbstractItemView::SingleSelection);samples_->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
 samples_->setStyleSheet("QListWidget{background:transparent;color:#1e395b;outline:0;} QListWidget::item{padding:0 2px;} QListWidget::item:selected{background:#9ecbf5;color:#1e395b;border-top:1px dotted #c7e0f7;border-bottom:1px dotted #c7e0f7;}");
 samples_->setContextMenuPolicy(Qt::CustomContextMenu);
 connect(samples_,&QListWidget::currentRowChanged,&c,&CalculatorController::selectSample);
 connect(samples_,&QListWidget::itemChanged,this,[this](QListWidgetItem*item){controller_.updateSample(samples_->row(item),item->text());});
 connect(up_,&QAbstractButton::clicked,this,[this]{controller_.selectSample(qMax(0,controller_.state().selectedSample-1));});
 connect(down_,&QAbstractButton::clicked,this,[this]{controller_.selectSample(qMin(count_-1,controller_.state().selectedSample+1));});
 setContextMenuPolicy(Qt::CustomContextMenu);
 connect(this,&QWidget::customContextMenuRequested,this,[this](QPoint p){showContextMenu(mapToGlobal(p));});
 connect(samples_,&QWidget::customContextMenuRequested,this,[this](QPoint p){showContextMenu(samples_->viewport()->mapToGlobal(p));});
}
void StatisticsControls::showContextMenu(QPoint globalPosition){
 QMenu menu(this);auto*copy=menu.addAction(tr("Copy"));
 auto*paste=menu.addAction(tr("Paste"));paste->setEnabled(!QApplication::clipboard()->text().isEmpty());
 menu.addSeparator();auto*clear=menu.addAction(tr("Clear dataset"));clear->setEnabled(count_>0);
 auto*action=menu.exec(globalPosition);
 if(action==copy)controller_.copy();else if(action==paste)controller_.paste();else if(action==clear)controller_.clearSamples();
}
void StatisticsControls::sync(const CalculatorState&s,qreal scale){
 setVisible(s.mode==Mode::Statistics);scale_=scale;count_=static_cast<int>(s.samples.size());
 setGeometry(scaledRect(QRect(11,30,190,150),scale));samples_->setGeometry(scaledRect(QRect(6,27,178,73),scale));
 up_->setGeometry(scaledRect(QRect(145,6,16,16),scale));down_->setGeometry(scaledRect(QRect(167,6,16,16),scale));
 auto f=QFont("Consolas");f.setPixelSize(qRound(11*scale));samples_->setFont(f);
 QSignalBlocker blocker(samples_);
 while(samples_->count()>count_)delete samples_->takeItem(samples_->count()-1);
 for(int i=0;i<count_;++i){
  if(i>=samples_->count()){auto*item=new QListWidgetItem(samples_);item->setFlags(item->flags()|Qt::ItemIsEditable);item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);}
  auto*item=samples_->item(i);const auto text=QString::fromStdWString(s.samples[i]);
  if(item->text()!=text)item->setText(text);item->setSizeHint(QSize(qRound(178*scale),qRound(14*scale)));
 }
 if(samples_->currentRow()!=s.selectedSample)samples_->setCurrentRow(s.selectedSample);
 if(s.selectedSample>=0)samples_->scrollToItem(samples_->item(s.selectedSample),QAbstractItemView::EnsureVisible);
 update();
}
void StatisticsControls::paintEvent(QPaintEvent*){
 QPainter p(this);p.scale(scale_,scale_);p.setPen(QColor("#cdd8e7"));p.drawLine(5,26,184,26);p.drawLine(5,100,184,100);p.setPen(Qt::white);p.drawLine(5,101,184,101);
 auto f=font();f.setPixelSize(9);p.setFont(f);p.setPen(LegacyPalette::keyText);p.drawText(QRect(5,103,178,14),Qt::AlignLeft|Qt::AlignVCenter,tr("Count = %1").arg(count_));
}
}
