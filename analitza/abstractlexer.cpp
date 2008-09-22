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

#include "abstractlexer.h"
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

QMap<QChar, int> AbstractLexer::m_operators=initializeOperators();
QMap<QString, int> AbstractLexer::m_longOperators=initializeLongOperators();

AbstractLexer::AbstractLexer(const QString &source)
	: current(-1, 0), m_source(source)
{}

AbstractLexer::~AbstractLexer() {}

void AbstractLexer::printQueue(const QQueue<TOKEN>& q) const
{
	QStringList res;
	foreach(const TOKEN& t, q)
	{
		if(m_longOperators.values().contains(t.type))  res += m_longOperators.key(t.type);
		else if(m_operators.values().contains(t.type)) res += m_operators.key(t.type);
		else res+= (t.val + error());
	}
	qDebug() << "(" << res.join(QString()) << ")";
}

int AbstractLexer::lex()
{
	if(m_tokens.isEmpty())
		getToken();
	
	Q_ASSERT(!m_tokens.isEmpty());
	current=m_tokens.takeFirst();
// 	printQueue(m_tokens);
	
	return current.type;
}
