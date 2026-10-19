// SPDX-License-Identifier: MIT
#include "KeyLayout.h"
namespace Win7Calc {
using C = CalculationManager::Command;
QSize clientSize(Mode mode,Panel panel,bool history,qreal scale) {
 int width=(mode==Mode::Scientific||mode==Mode::Programmer)?407:212;
 int height=mode==Mode::Programmer?350:mode==Mode::Statistics?384:284;
 if(history&&(mode==Mode::Standard||mode==Mode::Scientific))height+=100;
 if(panel!=Panel::Basic)width+=374;
 return {qRound(width*scale),qRound(height*scale)};
}
QRect scaledRect(QRect r,qreal s) {
 return {qRound(r.x()*s),qRound(r.y()*s),qRound((r.x()+r.width())*s)-qRound(r.x()*s),qRound((r.y()+r.height())*s)-qRound(r.y()*s)};
}
std::vector<KeySpec> keyLayout(Mode mode,bool) {
 if(mode!=Mode::Standard)return {};
 std::vector<KeySpec> result;
 const int xs[]={11,50,89,128,167}, widths[]={33,34,34,34,34};
 auto add=[&](QString id,QString label,QString name,int col,int row,C command,KeyAppearance appearance=KeyAppearance::Operator,int width=0,int height=27){
  result.push_back({id,label,name,QRect(xs[col],85+32*row,width?width:widths[col],height),command,appearance});
 };
 add("memory-clear","MC","Clear memory",0,0,C::CommandMCLEAR,KeyAppearance::Memory);
 add("memory-recall","MR","Recall memory",1,0,C::CommandRECALL,KeyAppearance::Memory);
 add("memory-store","MS","Store in memory",2,0,C::CommandSTORE,KeyAppearance::Memory);
 add("memory-add","M+","Add to memory",3,0,C::CommandMPLUS,KeyAppearance::Memory);
 add("memory-subtract","M-","Subtract from memory",4,0,C::CommandMMINUS,KeyAppearance::Memory);
 add("backspace","←","Backspace",0,1,C::CommandBACK);
 add("clear-entry","CE","Clear entry",1,1,C::CommandCENTR);
 add("clear","C","Clear",2,1,C::CommandCLEAR);
 add("negate","±","Change sign",3,1,C::CommandSIGN);
 add("square-root","√","Square root",4,1,C::CommandSQRT);
 for(int row=2;row<=4;++row)for(int col=0;col<3;++col){
  int digit=(4-row)*3+col+1;
  add(QString("digit-%1").arg(digit),QString::number(digit),QString::number(digit),col,row,static_cast<C>(130+digit),KeyAppearance::Number);
 }
 add("divide","/","Divide",3,2,C::CommandDIV);add("percent","%","Percent",4,2,C::CommandPERCENT);
 add("multiply","*","Multiply",3,3,C::CommandMUL);add("reciprocal","1/x","Reciprocal",4,3,C::CommandREC);
 add("subtract","-","Subtract",3,4,C::CommandSUB);add("equals","=","Equals",4,4,C::CommandEQU,KeyAppearance::Operator,34,59);
 add("digit-0","0","0",0,5,C::Command0,KeyAppearance::Number,73);
 add("decimal",".","Decimal point",2,5,C::CommandPNT,KeyAppearance::Number);add("add","+","Add",3,5,C::CommandADD);
 return result;
}
}
