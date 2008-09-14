/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@gmail.com>                             *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "explexer.h"
#include "expressionparser.h"
#include <QDebug>
#include <QStringList>

QMap<QChar, int> initializeOperators()
{
	QMap<QChar, int> operators;
	operators['+']=ExpressionTable::tAdd;
	operators['-']=ExpressionTable::tSub;
	operators['*']=ExpressionTable::tMul;
	operators['/']=ExpressionTable::tDiv;
	operators['^']=ExpressionTable::tPow;
	operators['(']=ExpressionTable::tLpr;
	operators[')']=ExpressionTable::tRpr;
	operators[',']=ExpressionTable::tComa;
	operators['{']=ExpressionTable::tLcb;
	operators['}']=ExpressionTable::tRcb;
	operators['?']=ExpressionTable::tQm;
	operators['|']=ExpressionTable::tBar;
	operators['=']=ExpressionTable::tEq;
	return operators;
}

QMap<QString, int> initializeLongOperators()
{
	QMap<QString, int> longOperators;
	longOperators["->"]=ExpressionTable::tLambda;
	longOperators[":="]=ExpressionTable::tAssig;
	longOperators[".."]=ExpressionTable::tLimits;
	longOperators["**"]=ExpressionTable::tPow;
	return longOperators;
}

QMap<QChar, int> ExpLexer::m_operators=initializeOperators();
QMap<QString, int> ExpLexer::m_longOperators=initializeLongOperators();
// QMap<int, QString> ExpLexer::m_operatorTags=initializeOperatorTags();

ExpLexer::ExpLexer(const QString &source)
	: m_source(source)
{
}

ExpLexer::~ExpLexer()
{
}

int ExpLexer::lex()
{
	int dontlookatme=-1;
	TOKEN c=getToken(m_source, dontlookatme);
// 	if(c.type==ExpressionTable::tVal)
	current=c;
	
// 	if(m_longOperators.values().contains(c.type))
// 		qDebug() << "-" << m_longOperators.key(c.type);
// 	else if(m_operators.values().contains(c.type))
// 		qDebug() << "-" << m_operators.key(c.type);
// 	else
// 		qDebug() << "-" << c.val << c.error;
	
	return c.type;
}

ExpLexer::TOKEN ExpLexer::getToken(QString &a, int &l)
{
	bool exp= (l<0);
	int i=0;
	l=a.length();
	a = a.trimmed();
	TOKEN ret;
	ret.type = -1;
	
	if(a.isEmpty())
		ret.type = ExpressionTable::EOF_SYMBOL;
	else if(a[0].decompositionTag()==QChar::Super) {
		for(int i=0; i<a.count() && a[i].decompositionTag()==QChar::Super; i++) {
			ret.type = ExpressionTable::tPow;
			a[i]=a[i].decomposition()[0];
		}
		a.prepend(' ');
	} else if(a[0].isDigit() || (a[0]=='.' && a[1].isDigit())) {
		int coma=0;
		
		for(i=0; a[i].isDigit() || (a[i]=='.' && a[i+1]!='.'); i++){
			if(a[i]=='.')
				coma++;
			ret.val += a[i];
			a[i]=' ';
		}
		for(; a[i].isSpace(); i++) {}
		
		if(exp && (a[i] == '(' || a[i].isLetter()))
			a.prepend(" *");
		
		QStringList attrib;
		if(coma)
			attrib+="type='real'";
		
		if(coma>1)
			ret.val="<cn>&error;</cn>";
		else if(attrib.isEmpty())
			ret.val = QString("<cn>%2</cn>").arg(ret.val);
		else
			ret.val = QString("<cn %1>%2</cn>").arg(attrib.join(" ")).arg(ret.val);
		ret.type= ExpressionTable::tVal;
	} else if(a[0].isLetter()) {//es una variable o funcret.val += a[0];
		for(i=0; a[i].isLetterOrNumber() && a[i].decompositionTag()==QChar::NoDecomposition; i++){
			ret.val += a[i];
			a[i]=' ';
		}
		
		for(; a[i].isSpace(); i++) {}
		
		if((a[i]=='(' || a[i].isLetterOrNumber()) && a[i].decompositionTag()==QChar::NoDecomposition) {
			ret.type=ExpressionTable::tFunc;
		} else if((a[i]=='{' || a[i].isLetterOrNumber()) && a[i].decompositionTag()==QChar::NoDecomposition) {
			ret.type=ExpressionTable::tBlock;
		} else {
			ret.val = QString("<ci>%1</ci>").arg(ret.val);
			ret.type= ExpressionTable::tVal;
		}
	} else if(a.length()>=2 && m_longOperators.contains(QString(a[0])+a[1])) {
		ret.type=m_longOperators[QString(a[0])+a[1]];
// 		ret.val=m_operatorTags[ret.type];
		a[0]=a[1]=' ';
	} else if(!a.isEmpty() && m_operators.contains(a[0])) {
		ret.type=m_operators[a[0]];
// 		ret.val=m_operatorTags[ret.type];
		a[0]=' ';
	} else {
		ret.val=-1;
		ret.error=QString("Unknown token %1").arg(a[0]);
	}
	
// 	qDebug() << "lalala" << a << ret.val;
	a=a.trimmed();
	l-=a.length();
	
	return ret;
}
