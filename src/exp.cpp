 /***************************************************************************
 *   Copyright (C) 2005 by aleix                                           *
 *   aleixpol@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "exp.h"
#include "operator.h"
using namespace std;

#if 0
QString opr2str(int in);
void printPilaOpr(QStack<int> opr);

QString opr2str(int in)
{
	QString ret;
	switch(in){
		case tAssig:	ret="tAssig";	break;
		case tLambda:	ret="tLambda";	break;
		case tLimits:	ret="tLimits";	break;
		case tAdd:	ret="tAdd";	break;
		case tSub:	ret="tSub";	break;
		case tMul:	ret="tMul";	break;
		case tDiv:	ret="tDiv";	break;
		case tPow:	ret="tPow";	break;
		case tUmi:	ret="tUmi";	break;
		case tFunc:	ret="tFunc";	break;
		case tComa:	ret="tComa";	break;
		case tLpr:	ret="tLpr";	break;
		case tRpr:	ret="tRpr";	break;
		case tEof:	ret="tEof";	break;
		case tMaxOp:	ret="tMaxOp";	break;
		case tVal:	ret="tVal";	break;
		case tVar:	ret="tVar";	break;
		default:	ret="chalaoooooooooo";
	}
	return ret;
}

void printPilaOpr(QStack<int> opr) //debug only
{
	bool istop=true;
	while(!opr.isEmpty()) {
		qDebug() << ":   " << opr2str(opr.pop());
		istop=false;
	}
}

void printPilaVal(QStack<QString> val)
{
	while(!val.isEmpty()) {
		qDebug() << ":   " << val.pop();
	}
}
#endif

const actEnum parseTbl[tMaxOp][tMaxOp] = {
//	 :=   ->  ..  +   -   *   /   ^   M   f   ,   (   )   $
	{ R,  S,  S,  S,  S,  S,  S,  S,  S,  S,  R,  S,  R,  R },	//:=
	{ R,  S,  S,  S,  S,  S,  S,  S,  S,  S,  R,  S,  R,  R },	//-> Lambda
	{ R,  R,  R,  S,  S,  S,  S,  S,  S,  S,  R,  S,  R,  R },	//.. Limits
	{ R,  R,  R,  K,  R,  S,  S,  S,  S,  S,  R,  S,  R,  R },	//+
	{ R,  R,  R,  R,  K,  S,  S,  S,  S,  S,  R,  S,  R,  R },	//-
	{ R,  R,  R,  R,  R,  K,  R,  S,  S,  S,  R,  S,  R,  R },	//*
	{ R,  R,  R,  R,  R,  R,  R,  S,  S,  S,  R,  S,  R,  R },	///
	{ R,  R,  R,  R,  R,  R,  R,  R,  S,  S,  R,  S,  R,  R },	//^
	{ R,  R,  R,  R,  R,  R,  R,  S,  S,  S,  R,  S,  R,  R },	//UnaryMinus
	{ R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  R,  S,  R,  R },	//func
	{ S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  R,  R,  R,  E },	//,
	{ S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  S, E1 },	//(
	{ R,  R,  R,  R,  R,  R,  R,  R,  R, E3,  R, E2,  R,  R },	//)
	{ S,  S,  S,  S,  S,  S,  S,  S,  S,  S,  E,  S, E3,  A },	//$
};

Exp::Exp(QString exp) : str(exp)
{}

Exp::~Exp(){}

TOKEN Exp::pillatoken(QString &a){
	int i=0;
	a = a.trimmed();
// 	printf("%s\n", a.ascii());
	TOKEN ret;
	ret.tipus = tMaxOp;
	if(a.isEmpty())
		ret.tipus = tEof;
	else if(a[0].isDigit() || (a[0]=='.' && a[1].isDigit())) {//es un numero
		int coma=0;
		if(a[0]=='.') {
			ret.val += "0";
			coma++;
		}
		ret.val += a[0];
		a[0]=' ';
		for(i=1; a[i].isDigit() || (a[i]=='.' && a[i+1]!='.'); i++){
			coma = (a[i]=='.')? coma+1 : coma;
			ret.val += a[i];
			a[i]=' ';
		}
		if(a[i] == '(' || a[i].isLetter())
			a.prepend(" *");
		
		if(coma>1)
			err << i18n("Too much comma in %1").arg(ret.val);
		
		ret.val = QString::QString("<cn>%1</cn>").arg(ret.val);
		ret.tipus= tVal;
	} else if(a[0].isLetter()) {//es una variable o func
		ret.val += a[0];
		for(i=1; a[i].isLetterOrNumber(); i++){
			ret.val += a[i];
			a[i]=' ';
		}
		
		for(;a[i].isSpace();i++);
		
		if(a[i]=='(' || a[i].isLetterOrNumber()) {
			ret.tipus=tFunc;
		} else {
			ret.val = QString::QString("<ci>%1</ci>").arg(ret.val);
			ret.tipus= tVal;
		}
	} else if(a[0]=='-' && a[1] == '>') {
		ret.tipus = tLambda;
		a[1] =' ';
	} else if(a[0]==':' && a[1] == '=') {
		ret.tipus = tAssig;
		a[1] =' ';
	} else if(a[0]=='.' && a[1] == '.') {
		ret.tipus = tLimits;
		a[1] =' ';
	} else if(a[0]=='+')
		ret.tipus = tAdd;
	else if(a[0]=='-')
		ret.tipus = (antnum == tVal || antnum==tRpr) ? tSub : tUmi;
	else if(a[0]=='/')
		ret.tipus = tDiv;
	else if(a[0]=='^')
		ret.tipus = tPow;
	else if(a[0]=='*' && a[1] == '*') {
		ret.tipus = tPow;
		a[1] =' ';
	} else if(a[0]=='*')
		ret.tipus = tMul;
	else if(a[0]=='(')
		ret.tipus = tLpr;
	else if(a[0]==')')
		ret.tipus = tRpr;
	else if(a[0]==',')
		ret.tipus = tComa;
	else
		err << i18n("Unknown token %1").arg(a[0]);
	
	a[0]=' ';
	antnum = ret.tipus;
	//qDebug() << ret.val;
	if(antnum==tok && tok<tLpr)
		err << i18n("Value remaining");
	return ret;
}

int Exp::getTok(){
	QString s;
	TOKEN t;
	if(firsttok)
		firsttok=false;
	
	t=pillatoken(str);
	tok = t.tipus;
	tokval = t.val;
	
	return 0;
}

int Exp::shift(){
// 	cout << "------>" << tokval.ascii() << "'" << endl;
	if(tok==tVal)
		val.push(tokval);
	else if(tok==tFunc){
		func.push(tokval);
		opr.push((char)tok);
	} else
		opr.push((char)tok);
	if(getTok())
		return 1;
	return 0;
}

int Exp::reduce(){
	tokEnum oper = (tokEnum) opr.top();
	opr.pop();
	QString aux;
	if(val.isEmpty()) {
		aux = "0";
		err << i18n("Value Stack is empty");
	} else
		aux = val.pop();
	
	switch(oper) {
		case tAdd:
			if(!val.isEmpty()) val.push(QString("<apply><plus />%1%2</apply>").arg(val.pop()).arg(aux));
			break;
		case tUmi:
			val.push(QString("<apply><minus />%1</apply>").arg(aux));
			break;
		case tSub:
			if(!val.isEmpty()) val.push(QString("<apply><minus />%1%2</apply>").arg(val.pop()).arg(aux));
			break;
		case tMul:
			if(!val.isEmpty()) val.push(QString("<apply><times />%1%2</apply>").arg(val.pop()).arg(aux));
			break;
		case tDiv:
			if(!val.isEmpty()) val.push(QString("<apply><divide />%1%2</apply>").arg(val.pop()).arg(aux));
			break;
		case tPow:
			if(!val.isEmpty()) val.push(QString("<apply><power />%1%2</apply>").arg(val.pop()).arg(aux));
			break;
		case tLimits:
			if(!val.isEmpty()) val.push(QString("<uplimit>%2</uplimit><downlimit>%1</downlimit>").arg(val.pop()).arg(aux));
			break;
		case tAssig: // :=
			if(!val.isEmpty()) val.push(QString("<declare>%1%2</declare>").arg(val.pop()).arg(aux));
			break;
		case tFunc:
			if(Operator::toOperatorType(func.top()) && !func.isEmpty())
				val.push(QString("<apply><%1 />%2</apply>").arg(func.pop()).arg(aux));
			else
				val.push(QString("<apply><ci type='function'>%1</ci>%2</apply>").arg(func.pop()).arg(aux));
			break;
		case tLambda: // ->
			if(!val.isEmpty() && opr.top()==tLambda || (opr.count()>1 && opr[1]==tFunc))
				val.push(QString("<bvar>%1</bvar>%2").arg(val.pop()).arg(aux));
			else if(!val.isEmpty())
				val.push(QString("<lambda><bvar>%1</bvar>%2</lambda>").arg(val.pop()).arg(aux));
			break;
		case tComa:
			if(!val.isEmpty()) val.push(QString("%1%2").arg(val.pop()).arg(aux));
			break;
		case tRpr:
			opr.pop();
		default:
			val.push(aux);
			break;
	}
	return 0;
}

int Exp::parse()
{
	opr.push(tEof);
	firsttok = true;
	antnum= tEof;
	tok=tEof;
	
	if(getTok()) return 1;
	while(err.isEmpty()){
		if(tok==tVal){
			if(shift()) return 1;
			continue;
		}
		QString a; //For PPC too
// 		printf("acc=%d stk=%d, tok=%d\n", parseTbl[opr.top()][tok], opr.top(), tok);
		switch(parseTbl[opr.top()][tok]){
			case K:
				a=val.pop();
				if(opr.top()==tLambda && !val.isEmpty())
					val.push(QString("%1<bvar>%2</bvar>").arg(val.pop()).arg(a));
				else if(!val.isEmpty())
					val.push(QString("%1%2").arg(val.pop()).arg(a));
				opr.pop();
				break;
			case R:
				if(reduce()) return 1;
				break;
			case S:
				if(shift()) return 1;
				break;
			case A:
// 				printPilaOpr(opr);
// 				printPilaVal(val);
				if(val.count()==1)
					mml = QString("<math>%1</math>").arg(val.pop());
				else if(val.count()>1)
					err << i18n("Wrong stack result: %1").arg(val.pop());
				else
					err << i18n("Wrong stack result");
				return 0;
			case E1:
				err << i18n("Missing right paranthesis");
				return 1;
			case E2:
				err << i18n("Missing operator");
				return 2;
			case E3:
				err << i18n("Unbalanced right parenthesis");
				return 3;
			case E:
				err << i18n("Misplaced coma");
				return 4;
			default:
				break;
		}
	}
	return -1;
}

QString Exp::mathML()
{
	return mml;
}

QStringList Exp::error()
{
	return err;
}
