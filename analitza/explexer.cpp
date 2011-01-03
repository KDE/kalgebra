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

#include "explexer.h"
#include "expressionparser.h"
#include <QDebug>
#include <QStringList>
#include <KLocale>

ExpLexer::ExpLexer(const QString &source)
	: AbstractLexer(source), m_pos(0)
	, m_realRx("^-?((\\.[0-9]+)|[0-9]+(\\.[0-9]+)?)(e-?[0-9]+)?", Qt::CaseSensitive, QRegExp::RegExp2)
{}

void ExpLexer::getToken()
{
	int& pos=m_pos;
	const QString& a=m_source;
	for(; pos<a.length() && a[pos].isSpace(); pos++) {}
	
	int oldpos=pos;
	TOKEN ret(-1, pos);
	
	if(pos>=a.length()) {
		ret.type = ExpressionTable::EOF_SYMBOL;
	} else if(a.length()>pos+1 && m_longOperators.contains(QString(a[pos])+a[pos+1])) {
		ret.type=m_longOperators[QString(a[pos])+a[pos+1]];
		pos+=2;
	} else if(m_operators.contains(a[pos])) {
		ret.type=m_operators[a[pos]];
		pos++;
    } else if(a[pos].decompositionTag()==QChar::Super) {
		QString super;
		for(int i=pos; i<a.count() && a[i].decompositionTag()==QChar::Super; i++) {
			ret.type = ExpressionTable::tPow;
			super+=a[i].decomposition()[0];
			
			pos++;
		}
		m_tokens.append(TOKEN(ExpressionTable::tPow, pos, QString(), 0));
		ret=TOKEN(ExpressionTable::tVal, oldpos, "<cn>"+super+"</cn>", pos-oldpos);
	} else if(a[pos].isLetter()) {
		for(; pos<a.length() && a[pos].isLetterOrNumber() && (a[pos].isLetter() || a[pos].decompositionTag()==QChar::NoDecomposition); pos++){
			ret.val += a[pos];
		}
		ret.type= ExpressionTable::tId;
		Q_ASSERT(!ret.val.isEmpty());
	} else {
		int l = m_realRx.indexIn(a, pos, QRegExp::CaretAtOffset);
		Q_ASSERT(l==pos || l==-1);
		
		if(l==pos) {
			ret.val = m_realRx.cap();
			
			QString attrib;
			if(!m_realRx.cap(2).isEmpty() || !m_realRx.cap(3).isEmpty() || !m_realRx.cap(4).isEmpty())
				attrib+=" type='real'";
			
			Q_ASSERT(!ret.val.isEmpty());
			
			ret.val = QString("<cn%1>%2</cn>").arg(attrib).arg(ret.val);
			ret.type= ExpressionTable::tVal;
			
			pos += m_realRx.matchedLength();
		} else {
			ret.val=-1;
			m_err=i18n("Unknown token %1", a[pos]);
		}
	}
	ret.len = pos-oldpos;
	m_tokens.append(ret);
	
// 	printQueue(m_tokens);
}
