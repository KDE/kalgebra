/*************************************************************************************
 *  Copyright (C) 2008 by Aleix Pol <aleixpol@kde.org>                               *
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
	operators['[']=ExpressionTable::tLsp;
	operators[']']=ExpressionTable::tRsp;
	operators['?']=ExpressionTable::tQm;
	operators[':']=ExpressionTable::tColon;
	operators['=']=ExpressionTable::tEq;
	operators['<']=ExpressionTable::tLt;
	operators['>']=ExpressionTable::tGt;
	operators['@']=ExpressionTable::tAt;
	operators['|']=ExpressionTable::tPipe;
	return operators;
}

QMap<QString, int> initializeLongOperators()
{
	QMap<QString, int> longOperators;
	longOperators["->"]=ExpressionTable::tLambda;
	longOperators[":="]=ExpressionTable::tAssig;
	longOperators[".."]=ExpressionTable::tLimits;
	longOperators["**"]=ExpressionTable::tPow;
	longOperators["<="]=ExpressionTable::tLeq;
	longOperators[">="]=ExpressionTable::tGeq;
	longOperators["!="]=ExpressionTable::tNeq;
	return longOperators;
}

QMap<QChar, int> AbstractLexer::m_operators=initializeOperators();
QMap<QString, int> AbstractLexer::m_longOperators=initializeLongOperators();

AbstractLexer::AbstractLexer(const QString &source)
	: current(-1, 0), m_source(source), m_lines(0), m_openPr(0), m_openCb(0)
{}

AbstractLexer::~AbstractLexer() {}

void AbstractLexer::printQueue(const QQueue<TOKEN>& q) const
{
	QStringList res;
	foreach(const TOKEN& t, q)
	{
		if(m_longOperators.values().contains(t.type))  res += m_longOperators.key(t.type);
		else if(m_operators.values().contains(t.type)) res += m_operators.key(t.type);
		else res+= (t.val + ';' + QString::number(t.type) + error());
	}
	qDebug() << q.count() << ":::" << "(" << res.join("|") << ")";
}

int AbstractLexer::lex()
{
	if(m_tokens.isEmpty())
		getToken();
	
// 	printQueue(m_tokens);
	
	Q_ASSERT(!m_tokens.isEmpty());
	current=m_tokens.takeFirst();
	
	switch(current.type) {
		case ExpressionTable::tLpr:
			m_openPr++;
			break;
		case ExpressionTable::tRpr:
			m_openPr--;
			break;
		case ExpressionTable::tLcb:
			m_openCb++;
			break;
		case ExpressionTable::tRcb:
			m_openCb--;
			break;
		default:
			break;
	}
	
	return current.type;
}
