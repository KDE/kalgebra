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

#include "mathmlpresentationlexer.h"
#include "expressionparser.h"
#include <QDebug>
#include <QStringList>
#include <KLocale>

MathMLPresentationLexer::MathMLPresentationLexer(const QString &source)
	: AbstractLexer(source), m_xml(source)
{
	m_tokenTags["mfrac"]=TOKEN(ExpressionTable::tDiv, 0, "divide");
	m_tokenTags["msup"]=TOKEN(ExpressionTable::tPow, 0, "power");
	m_tokenTags["msqrt"]=TOKEN(ExpressionTable::tPow, 0, "root");
}

void MathMLPresentationLexer::getToken()
{
	while(!m_xml.atEnd()) {
		QXmlStreamReader::TokenType t=m_xml.readNext(); 
		QString e=m_xml.name().toString();
		switch(t)
		{
			case QXmlStreamReader::Invalid:
				m_err = m_xml.errorString();
				break;
			case QXmlStreamReader::StartDocument:
				break;
			case QXmlStreamReader::EndDocument:
				break;
			case QXmlStreamReader::StartElement:
				m_tags.push(e);
				if(m_tokenTags.contains(e)) {
					TOKEN t=m_tokenTags[e];
					if(t.type>=0) {
						m_tokens.append(TOKEN(ExpressionTable::tId, 0, t.val));
					}
					
					m_tokens.append(TOKEN(ExpressionTable::tLpr, 0));
				}
				break;
			case QXmlStreamReader::EndElement:
				if(m_tokenTags.contains(e)) {
					if(m_tokens.last().type==ExpressionTable::tComa)
						m_tokens.takeLast();
					m_tokens.append(TOKEN(ExpressionTable::tRpr, 0));
				}
				
				m_tags.pop();
				if(!m_tags.isEmpty() && m_tokenTags.contains(m_tags.top()) && e!="mo") {
					m_tokens.append(TOKEN(ExpressionTable::tComa, 0));
				}
				break;
			default:
				if(m_tags.top()=="mn") {
					TOKEN ret(ExpressionTable::tVal, 0, QString("<cn>%1</cn>").arg(m_xml.text().toString()));
					m_tokens.append(ret);
				} else if(m_tags.top()=="mi") {
					TOKEN ret(ExpressionTable::tId, 0, m_xml.text().toString());
					m_tokens.append(ret);
				} else if(m_tags.top()=="mo") {
					if(!m_tokens.isEmpty() && m_tokens.last().type==ExpressionTable::tComa)
						m_tokens.takeLast();
					
					int t;
					QString op=m_xml.text().toString();
					if(op.length()==1 && m_operators.contains(op[0]))
						t = m_operators[op[0]];
					else if(op.length()==2 && m_longOperators.contains(op))
						t = m_longOperators[op];
					else
						m_err= i18nc("Error message", "Unknown token %1", op);
					m_tokens.append(TOKEN(t, 0));
				} else
					qDebug() << "dunno" << m_xml.text().toString();
				break;
		}
	}
	
// 	printQueue(m_tokens);
	
	if (m_xml.error())
		m_err = m_xml.errorString();
	
	m_tokens.append(TOKEN(ExpressionTable::EOF_SYMBOL, 0));
}
