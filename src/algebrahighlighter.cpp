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

#include "algebrahighlighter.h"
#include "expressionparser.h"
#include <QApplication>

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

QString removeTags(const QString& in)
{
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
	QColor number(pal.color(QPalette::Active, QPalette::Link));
	QColor variable(pal.color(QPalette::Active, QPalette::LinkVisited));
	QColor id(150,0,50);
	QColor uncorrect(Qt::red);
	QColor brHighlight(0xff,0xa0,0xff);
	QColor prHighlight(0xff,0xff,0x80);
	
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
					setFormat(i+1, j-i-1, id);
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
		
		m_correct=(inside==0);
	} else {
		ExpLexer lex(text);
		
		while(lex.lex()!=ExpressionTable::EOF_SYMBOL && lex.error().isEmpty()) {
			QColor f;
			bool isBold=false;
			switch(lex.current.type){
				case ExpressionTable::tVal:
					if(lex.current.val.mid(1,2)=="cn") //if it is a number
						f=number;
					else { //if it is a variable
						if(a && a->variables()->contains(removeTags(lex.current.val)))
							f=number;
						else
							f=variable;
					}
					break;
				case ExpressionTable::tId:
					f=id;
					break;
				case -1:
					m_correct = false;
					f=uncorrect;
					break;
				default:
					isBold=true;
					setFormat(lex.current.pos, lex.current.len, negreta);
					break;
			}
			if(!isBold)
				setFormat(lex.current.pos, lex.current.len, f);
		}
		
		if(!lex.error().isEmpty())
		{
			setFormat(lex.current.pos, text.size()-lex.current.pos, uncorrect);
		}
		
		bgHighlight(text, prHighlight, Parenthesis);
		bgHighlight(text, brHighlight, Brace);
	}
}

void AlgebraHighlighter::bgHighlight(const QString& text, const QColor& bgColor, ComplMode c)
{
	int p=-1;
	if(m_pos>0 && (
		(c==Parenthesis && (text[m_pos-1]=='(' || text[m_pos-1]==')')) ||
		(c==Brace && (text[m_pos-1]=='{' || text[m_pos-1]=='}'))
	))
		p=m_pos-1;
	else if(text.length()>m_pos && (
		(c==Parenthesis && (text[m_pos]=='(' || text[m_pos]==')')) ||
		(c==Brace && (text[m_pos]=='{' || text[m_pos]=='}'))
	))
		p=m_pos;
	
	if(p>-1) {
		QTextCharFormat form = format(p);
		form.setBackground(bgColor);
		setFormat(p, 1, form);
		int compp=complementary(text, p, c);
		if(compp==-3)
			form.setBackground(Qt::red);
		setFormat(p, 1, form);
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
	
	if(cat>0)
		return -2;
	else if(cat<0)
		return -3;
	return p;
}
