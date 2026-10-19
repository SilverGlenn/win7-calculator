// SPDX-License-Identifier: MIT
#include "CalculatorWindow.h"
#include "LegacyMetrics.h"
#include "LegacyPalette.h"
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QIcon>
#include <QLinearGradient>
#include <QMessageBox>
#include <QPainter>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QResource>
// Explicitly reference the resource object so static-library consumers retain it.
static void initializeCalculatorResources() { Q_INIT_RESOURCE(resources); }
namespace Win7Calc {
CalculatorWindow::CalculatorWindow(CalculatorController& c,QWidget* p)
 :QWidget(p),controller_(c),menus_(new QMenuBar(this)),display_(new CalculatorDisplay(this)),keypad_(new Keypad(c,this)),scientific_(new ScientificControls(c,keypad_)),programmer_(new ProgrammerControls(c,keypad_)),statistics_(new StatisticsControls(c,this)){
 initializeCalculatorResources();
 setObjectName("calculator-window");setWindowTitle(tr("Calculator"));
 setWindowIcon(QIcon::fromTheme("accessories-calculator",QIcon(":/icons/calculator.svg")));
 setFocusPolicy(Qt::StrongFocus);
 scale_=LegacyMetrics::fontScale(font());
 buildMenus();menus_->raise();display_->raise();statistics_->raise();
 connect(&controller_,&CalculatorController::stateChanged,this,&CalculatorWindow::refresh);
 connect(&controller_,&CalculatorController::invalidInput,this,[]{QApplication::beep();});
 connect(QApplication::clipboard(),&QClipboard::dataChanged,this,[this]{findChild<QAction*>("paste")->setEnabled(!QApplication::clipboard()->text().isEmpty());});
 refresh();
}
void CalculatorWindow::buildMenus(){
 menus_->setObjectName("menu-bar");menus_->setNativeMenuBar(false);
 menus_->setStyleSheet("QMenuBar { padding-left:2px; spacing:0px; border-top:1px solid #ffffff; border-bottom:1px solid #b7bed0; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #fdfdff,stop:0.3 #e4e9f5,stop:0.31 #d3daed,stop:1 #e4e7f1); } QMenuBar::item { background:transparent; padding:0px 5px; } QMenuBar::item:selected { border:1px solid #7b94b5; background:#dbe7f7; padding:0px 4px; }");
 auto* view=menus_->addMenu(tr("&View"));auto* modeGroup=new QActionGroup(this);
 const QStringList modes={tr("&Standard"),tr("S&cientific"),tr("&Programmer"),tr("S&tatistics")};
 for(int i=0;i<modes.size();++i){
  auto* a=view->addAction(modes[i]);a->setObjectName(QString("mode-%1").arg(i));a->setCheckable(true);a->setChecked(i==0);a->setShortcut(QKeySequence(QString("Alt+%1").arg(i+1)));a->setEnabled(true);modeGroup->addAction(a);
  connect(a,&QAction::triggered,this,[this,i]{controller_.setMode(static_cast<Mode>(i));});
 }
 view->addSeparator();
 auto* history=view->addAction(tr("&History"));history->setObjectName("history-toggle");history->setCheckable(true);history->setShortcut(QKeySequence("Ctrl+H"));history->setEnabled(false);
 auto* grouping=view->addAction(tr("&Digit grouping"));grouping->setObjectName("digit-grouping");grouping->setCheckable(true);connect(grouping,&QAction::triggered,&controller_,&CalculatorController::setDigitGrouping);
 view->addSeparator();auto* panelGroup=new QActionGroup(this);
 const QStringList panels={tr("&Basic"),tr("&Unit conversion"),tr("Date &calculation")};
 const QStringList shortcuts={"Ctrl+F4","Ctrl+U","Ctrl+E"};
 for(int i=0;i<3;++i){auto*a=view->addAction(panels[i]);a->setObjectName(QString("panel-%1").arg(i));a->setCheckable(true);a->setChecked(i==0);a->setShortcut(QKeySequence(shortcuts[i]));a->setEnabled(i==0);panelGroup->addAction(a);connect(a,&QAction::triggered,this,[this,i]{controller_.setPanel(static_cast<Panel>(i));});}
 auto* worksheets=view->addMenu(tr("&Worksheets"));
 for(int i=3;i<7;++i){auto*a=worksheets->addAction(QStringList{tr("Mortgage"),tr("Vehicle lease"),tr("Fuel economy (mpg)"),tr("Fuel economy (L/100 km)")}[i-3]);a->setObjectName(QString("panel-%1").arg(i));a->setCheckable(true);a->setEnabled(false);panelGroup->addAction(a);connect(a,&QAction::triggered,this,[this,i]{controller_.setPanel(static_cast<Panel>(i));});}
 auto* edit=menus_->addMenu(tr("&Edit"));
 auto* copy=edit->addAction(tr("&Copy"));copy->setObjectName("copy");copy->setShortcut(QKeySequence::Copy);connect(copy,&QAction::triggered,&controller_,&CalculatorController::copy);
 auto* paste=edit->addAction(tr("&Paste"));paste->setObjectName("paste");paste->setShortcut(QKeySequence::Paste);paste->setEnabled(!QApplication::clipboard()->text().isEmpty());connect(paste,&QAction::triggered,&controller_,&CalculatorController::paste);
 edit->addSeparator();auto* h=edit->addMenu(tr("&History"));
 for(const auto& text:{tr("Copy history"),tr("Edit"),tr("Cancel edit"),tr("Clear")}){auto*a=h->addAction(text);a->setEnabled(false);}
 auto* help=menus_->addMenu(tr("&Help"));auto* usage=help->addAction(tr("View &Help"));usage->setShortcut(QKeySequence(Qt::Key_F1));connect(usage,&QAction::triggered,this,&CalculatorWindow::showHelp);
 help->addSeparator();auto* about=help->addAction(tr("&About Calculator"));connect(about,&QAction::triggered,this,[this]{QMessageBox::about(this,tr("About Calculator"),tr("Windows 7 Calculator for Linux\nAn independent recreation using Microsoft's MIT-licensed calculation engine.\nNot a Microsoft Windows application."));});
 auto* modern=menus_->addMenu(tr("&Modern"));modern->addAction(tr("No modern features enabled"))->setEnabled(false);
}
void CalculatorWindow::refresh(){
 const auto& state=controller_.state();
 setFixedSize(clientSize(state.mode,state.panel,state.historyVisible,scale_));
 menus_->setGeometry(0,0,width(),qRound(19*scale_));
 keypad_->setGeometry(rect());keypad_->sync(state,scale_);scientific_->sync(state,scale_);programmer_->sync(state,scale_);
 for(int i=0;i<4;++i)findChild<QAction*>(QString("mode-%1").arg(i))->setChecked(i==static_cast<int>(state.mode));
 int baseWidth=(state.mode==Mode::Scientific||state.mode==Mode::Programmer)?407:212;
 int dh=state.mode==Mode::Statistics||(state.historyVisible&&controller_.historyAvailable())?150:50;
 display_->setGeometry(scaledRect(QRect(11,30,baseWidth-22,dh),scale_));
 display_->setState(state.display,scale_);statistics_->sync(state,scale_);
 findChild<QAction*>("digit-grouping")->setChecked(state.digitGrouping);
 findChild<QAction*>("history-toggle")->setChecked(state.historyVisible);
 update();
}
void CalculatorWindow::paintEvent(QPaintEvent*){
 QPainter p(this);QLinearGradient bg(0,qRound(19*scale_),0,qRound(110*scale_));bg.setColorAt(0,LegacyPalette::backgroundTop);bg.setColorAt(1,LegacyPalette::backgroundBottom);p.fillRect(rect(),bg);
}
void CalculatorWindow::keyPressEvent(QKeyEvent* event){if(!controller_.handleKey(event))QWidget::keyPressEvent(event);}
void CalculatorWindow::showHelp(){
 auto* dialog=new QDialog(this);dialog->setAttribute(Qt::WA_DeleteOnClose);dialog->setWindowTitle(tr("Calculator Help"));dialog->resize(500,380);
 auto* layout=new QVBoxLayout(dialog);auto* text=new QTextBrowser(dialog);QFile file(":/help.html");if(file.open(QIODevice::ReadOnly))text->setHtml(QString::fromUtf8(file.readAll()));layout->addWidget(text);
 auto* buttons=new QDialogButtonBox(QDialogButtonBox::Close,dialog);connect(buttons,&QDialogButtonBox::rejected,dialog,&QDialog::close);layout->addWidget(buttons);dialog->show();
}
}
