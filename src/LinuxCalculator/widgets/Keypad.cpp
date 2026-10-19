// SPDX-License-Identifier: MIT
#include "Keypad.h"
namespace Win7Calc {
Keypad::Keypad(CalculatorController& c,QWidget* p):QWidget(p),controller_(c){setObjectName("keypad");}
void Keypad::sync(const CalculatorState& state,qreal scale) {
 if(mode_!=state.mode){
  for(auto* k:keys_)delete k;keys_.clear();mode_=state.mode;inverse_=state.inverse;
  for(const auto& spec:keyLayout(mode_,inverse_)){
   auto* key=new CalculatorKey(spec,this);keys_.push_back(key);
   if(spec.command)connect(key,&QAbstractButton::clicked,this,[this,cmd=*spec.command]{controller_.send(cmd);});
   key->show();
  }
 }
 inverse_=state.inverse;
 auto layout=keyLayout(mode_,inverse_);
 const int dy=state.historyVisible&&(mode_==Mode::Standard||mode_==Mode::Scientific)?100:0;
 for(std::size_t i=0;i<keys_.size();++i){
  if(state.mode==Mode::Programmer&&layout[i].command){
   const int digit=static_cast<int>(*layout[i].command)-130;
   if(digit>=0&&digit<16)layout[i].enabled=digit<state.radix;
  }
  if(state.display.error&&layout[i].id!="clear"&&layout[i].id!="clear-entry")layout[i].enabled=false;
  keys_[i]->setSpec(layout[i]);
  keys_[i]->setGeometry(scaledRect(layout[i].rect.translated(0,dy),scale));
  keys_[i]->setLogicalScale(scale);
 }
}
}
