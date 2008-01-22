/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@gmail.com>                             *
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

#include <QApplication>

#include "algebrahighlighter.h"
#include "expression.h"
#include "expressionedit.h"
#include "analitza.h"
#include "variables.h"
#include "container.h"

AlgebraHighlighter::AlgebraHighlighter(QTextDocument *doc, const Analitza *na)
	: QSyntaxHighlighter(doc), m_correct(true), m_mode(Autodetect), m_pos(0), a(na)
{
	negreta.setFontWeight(QFont::Bold);
}

QString removeTags(const QString& in){
	bool tag=false;
	QString out;
	for(int i=0; i<in.length(); i++){
		if(!tag && in[i]=='<')
			tag=true;
		else if(tag && in[i]=='>')
			tag=false;
		else if(!tag) {
			if(in.mid(i,4)=="&gt;"){
				out += '>';
				i+=3;
			} else
				out += in[i];
		}
	}
	return out;
}


void AlgebraHighlighter::highlightBlock(const QString &text)
{
	m_correct=true;
	if(m_pos>=text.length())
		m_pos=text.length();
	
	QPalette pal=qApp->palette();
		
	const QColor number(pal.color(QPalette::Active, QPalette::Link));
	const QColor variable(pal.color(QPalette::Active, QPalette::LinkVisited));
	const QColor definedFunction(0,50,0);
	const QColor undefinedFunction(0,0x86,0);
	const QColor block(50,0,50);
	const QColor uncorrect(Qt::red);
	
	if(Expression::isMathML(text)) {
		QString lasttag;
		int inside=0;
		for(int i=0; i<text.length(); i++) {
			if(text[i]=='<') { //We enter in a tag
				lasttag.clear();
				int j=i+1, k=0;
				for(k=i+1; k<text.length() && text[k]!='>'; k++){
					lasttag.append(text[k]);
					if(text[k]!=' ' && j==k-1)
						j=k;
				}
				j++;
				
				setFormat(i, 1, negreta);
				setFormat(j, 1, negreta);
				if(lasttag.startsWith(QChar('/'))){
					setFormat(i+1, j-i-1, QColor(100,0,0));
					setFormat(i+1, 1, negreta);
					inside--;
				} else if(lasttag.endsWith(QChar('/'))) {
					setFormat(i+1, j-i-1, definedFunction);
					setFormat(j+1, 2, negreta);
				} else if(j!=k) {
					setFormat(i+1, j-i-1, QColor(150,0,0));
					setFormat(j+1, k-j-1, QColor(150,100,0));
					inside++;
				} else {
					setFormat(i+1, j-i-1, QColor(150,0,0));
					inside++;
				}
				i=k;
			}
			else if(lasttag=="cn")
				setFormat(i, 1, number);
			else if(lasttag=="ci")
				setFormat(i, 1, variable);
		}
		if(inside==0)
			m_correct=true;
		else
			m_correct=false;
	} else {
		int pos=0, len=0;
		
		QString op=text.trimmed();
		TOKEN t=Exp::getToken(op, len, tMaxOp);
		for(pos=0; pos<text.length() && text[pos].isSpace(); pos++);
		
		while(pos < text.length() && t.type!=tEof){
			QColor f;
			bool isBold=false;
			switch(t.type){
				case tVal:
					if(t.val.mid(1,2)=="cn") //if it is a number
						f=number;
					else { //if it is a variable
						if(a && a->variables()->contains(removeTags(t.val)))
							f=number;
						else
							f=variable;
					}
					break;
				case tFunc:
					if(a && a->variables()->contains(t.val))
						f=definedFunction;
					else
						f=undefinedFunction;
					break;
				case tBlock:
					if(Container::toContainerType(t.val))
						f=block;
					else
						f=uncorrect;
					break;
				case tMaxOp:
					m_correct = false;
					f=uncorrect;
					break;
				default:
					isBold=true;
					setFormat(pos, len, negreta);
					break;
			}
			if(!isBold)
				setFormat(pos, len, f);
			pos += len;
			t=Exp::getToken(op, len, t.type);
		}
		
		//To bg highlight the parentheses
		int p=-1;
		if(m_pos>0 && (text[m_pos-1]=='(' || text[m_pos-1]==')'))
			p=m_pos-1;
		else if(text.length()>m_pos && (text[m_pos]=='(' || text[m_pos]==')'))
			p=m_pos;
		
		if(p>-1) {
			QTextCharFormat form = format(p);
			form.setBackground(QColor(0xff,0xff,0x80));
			setFormat(p, 1, form);
			if((p=complementary(text, p, Parenthesis))>=0)
				setFormat(p, 1, form);
		}
		
		//To bg highlight braces
		p=-1;
		if(m_pos>0 && (text[m_pos-1]=='{' || text[m_pos-1]=='}'))
			p=m_pos-1;
		else if(text.length()>m_pos && (text[m_pos]=='{' || text[m_pos]=='}'))
			p=m_pos;
		
		if(p>-1) {
			QTextCharFormat form = format(p);
			form.setBackground(QColor(0xff,0xa0,0xff));
			setFormat(p, 1, form);
			if((p=complementary(text, p, Brace))>=0)
				setFormat(p, 1, form);
		}
	}
}

int AlgebraHighlighter::complementary(const QString& t, int p, ComplMode m)
{
	char open, close;
	if(m==Brace) {
		open='{'; close='}';
	} else {
		open='('; close=')';
	}
	
	Q_ASSERT(p<t.length());
	bool opening = (t[p]==open);
	int cat=0;
	if(p<1 && !opening)
		return -1;
	do {
		if(t[p]==close)
			--cat;
		else if(t[p]==open)
			++cat;
		
		p += opening ? 1 : -1;
	} while(p>=0 && p<t.length() && cat!=0);
	
	p -= opening ? 1 : -1;
	
	if(cat!=0)
		return -2;
	return p;
}
