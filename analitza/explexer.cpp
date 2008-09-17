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
#include <KLocale>

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
	operators[':']=ExpressionTable::tColon;
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
	: current(-1, 0), m_pos(0), m_source(source)
{}

ExpLexer::~ExpLexer() {}

int ExpLexer::lex()
{
	Q_ASSERT(m_err.isEmpty());
	if(m_tokens.isEmpty())
		getToken(m_source, m_pos);
	
	Q_ASSERT(!m_tokens.isEmpty());
	current=m_tokens.takeFirst();
	
// 	if(m_longOperators.values().contains(current.type))  qDebug() << "-" << m_longOperators.key(current.type);
// 	else if(m_operators.values().contains(current.type)) qDebug() << "-" << m_operators.key(current.type);
// 	else qDebug() << "-" << current.val << error();
	
	return current.type;
}

void ExpLexer::getToken(const QString &a, int &pos)
{
	for(; pos<a.length() && a[pos].isSpace(); pos++) {}
	
	int oldpos=pos;
	TOKEN ret(-1, pos);
	
	if(pos==a.length()) {
		ret.type = ExpressionTable::EOF_SYMBOL;
	} else if(a[pos].decompositionTag()==QChar::Super) {
		QString super;
		for(int i=pos; i<a.count() && a[i].decompositionTag()==QChar::Super; i++) {
			ret.type = ExpressionTable::tPow;
			super+=a[i].decomposition()[0];
			
			pos++;
		}
		m_tokens.append(TOKEN(ExpressionTable::tPow, pos, QString(), 0));
		ret=TOKEN(ExpressionTable::tVal, oldpos, "<cn>"+super+"</cn>", pos-oldpos);
	} else if(a[pos].isDigit() || (a[pos]=='.' && a[1].isDigit())) {
		int coma=0;
		
		int i;
		for(i=pos; i<a.length() && (a[i].isDigit() || (a[i]=='.' && a[i+1]!='.')); i++){
			if(a[i]=='.')
				coma++;
			ret.val += a[i];
			pos++;
		}
		for(; i<a.length() && a[i].isSpace(); i++) {}
		
		if(i<a.length() && (a[i] == '(' || a[i].isLetter()))
			m_tokens.append(TOKEN(ExpressionTable::tMul, 0));
		
		QStringList attrib;
		if(coma)
			attrib+="type='real'";
		
		if(coma>1)
			m_err = i18nc("Error message", "Trying to codify an unknown value: %1", ret.val);
		else if(attrib.isEmpty())
			ret.val = QString("<cn>%2</cn>").arg(ret.val);
		else
			ret.val = QString("<cn %1>%2</cn>").arg(attrib.join(" ")).arg(ret.val);
		ret.type= ExpressionTable::tVal;
	} else if(a[pos].isLetter()) {//es una variable o funcret.val += a[0];
		int i;
		for(i=pos; i<a.length() && a[i].isLetterOrNumber() && a[i].decompositionTag()==QChar::NoDecomposition; i++){
			ret.val += a[i];
			pos++;
		}
		
		for(; i<a.length() && a[i].isSpace(); i++) {}
		
		if(i<a.length() && (a[i]=='(' || a[i].isLetterOrNumber()) && a[i].decompositionTag()==QChar::NoDecomposition) {
			ret.type=ExpressionTable::tFunc;
		} else if(i<a.length() && a[i]=='{' && a[i].decompositionTag()==QChar::NoDecomposition) {
			ret.type=ExpressionTable::tBlock;
		} else {
			ret.val = QString("<ci>%1</ci>").arg(ret.val);
			ret.type= ExpressionTable::tVal;
		}
	} else if(a.length()>pos+1 && m_longOperators.contains(QString(a[pos])+a[pos+1])) {
		ret.type=m_longOperators[QString(a[pos])+a[pos+1]];
		pos+=2;
	} else if(a.length()>pos && m_operators.contains(a[pos])) {
		ret.type=m_operators[a[pos]];
		pos++;
	} else {
		ret.val=-1;
		m_err=i18n("Unknown token %1", a[pos]);
	}
	ret.len = pos-oldpos;
	m_tokens.append(ret);
}
