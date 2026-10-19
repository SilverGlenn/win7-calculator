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
std::vector<KeySpec> keyLayout(Mode mode,bool inverse) {
 if(mode==Mode::Scientific){
  auto result=keyLayout(Mode::Standard,false);
  for(auto& k:result)k.rect.translate(195,0);
  auto add=[&](QString id,QString label,QString name,int col,int row,C command){
   const int xs[]={11,50,89,128,167}, widths[]={33,34,34,34,34};
   KeySpec spec{id,label,name,QRect(xs[col],85+32*row,widths[col],27),command};
   if(label.contains("⁻¹"))spec.fontPixels=10;
   result.push_back(spec);
  };
  KeySpec blank{"scientific-blank","","",QRect(11,117,33,27),std::nullopt,KeyAppearance::Memory};
  blank.enabled=false;result.push_back(blank);
  add("inverse","Inv","Inverse functions",1,1,C::CommandINV);
  result.back().checkable=true;result.back().checked=inverse;
  add("ln",inverse?"eˣ":"ln",inverse?"Exponential":"Natural logarithm",2,1,C::CommandLN);
  add("open-parenthesis","(","Open parenthesis",3,1,C::CommandOPENP);
  add("close-parenthesis",")","Close parenthesis",4,1,C::CommandCLOSEP);
  add("int",inverse?"Frac":"Int",inverse?"Fractional part":"Integer part",0,2,C::CommandCHOP);
  add("sinh",inverse?"sinh⁻¹":"sinh",inverse?"Inverse hyperbolic sine":"Hyperbolic sine",1,2,C::CommandSINH);
  add("sin",inverse?"sin⁻¹":"sin",inverse?"Inverse sine":"Sine",2,2,C::CommandSIN);
  add("square","x²","Square",3,2,C::CommandSQR);
  add("factorial","n!","Factorial",4,2,C::CommandFAC);
  add("dms",inverse?"deg":"dms",inverse?"Convert DMS to degrees":"Degrees minutes seconds",0,3,C::CommandDMS);
  add("cosh",inverse?"cosh⁻¹":"cosh",inverse?"Inverse hyperbolic cosine":"Hyperbolic cosine",1,3,C::CommandCOSH);
  add("cos",inverse?"cos⁻¹":"cos",inverse?"Inverse cosine":"Cosine",2,3,C::CommandCOS);
  add("power","xʸ","Power",3,3,C::CommandPWR);
  add("root","ʸ√x","Root",4,3,C::CommandROOT);
  add("pi",inverse?"2*π":"π",inverse?"Two pi":"Pi",0,4,C::CommandPI);
  add("tanh",inverse?"tanh⁻¹":"tanh",inverse?"Inverse hyperbolic tangent":"Hyperbolic tangent",1,4,C::CommandTANH);
  add("tan",inverse?"tan⁻¹":"tan",inverse?"Inverse tangent":"Tangent",2,4,C::CommandTAN);
  add("cube","x³","Cube",3,4,C::CommandCUB);
  add("cube-root","³√x","Cube root",4,4,C::CommandCUBEROOT);
  add("scientific-format","F-E","Scientific notation",0,5,C::CommandFE);
  add("exponent","Exp","Enter exponent",1,5,C::CommandEXP);
  add("modulo","Mod","Modulo",2,5,C::CommandMOD);
  add("log","log","Base ten logarithm",3,5,C::CommandLOG);
  add("power-ten","10ˣ","Power of ten",4,5,C::CommandPOW10);
  return result;
 }
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
