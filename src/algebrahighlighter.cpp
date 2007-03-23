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

AlgebraHighlighter::AlgebraHighlighter(QTextDocument *doc)
	: QSyntaxHighlighter(doc), m_wrong(false), m_mode(Autodetect), m_pos(0)
{
	negreta.setFontWeight(QFont::Bold);
}

void AlgebraHighlighter::highlightBlock(const QString &text)
{
	m_wrong=false;
	if(m_pos>=text.length())
		m_pos=text.length();
	if(Expression::isMathML(text)) {
		QString lasttag;
		for(int i=0; i<text.length(); i++){
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
				if(lasttag.startsWith("/")){
					setFormat(i+1, j-i-1, QColor(100,0,0));
					setFormat(i+1, 1, negreta);
				} else if(lasttag.endsWith("/")) {
					setFormat(i+1, j-i-1, QColor(0,50,0));
					setFormat(j+1, 2, negreta);
				} else if(j!=k) {
					setFormat(i+1, j-i-1, QColor(150,0,0));
					setFormat(j+1, k-j-1, QColor(150,100,0));
				} else
					setFormat(i+1, j-i-1, QColor(150,0,0));
				i=k;
			}
			else if(lasttag=="cn")
				setFormat(i, 1, QColor(0,0,200));
			else if(lasttag=="ci")
				setFormat(i, 1, QColor(100,0,0));
		}
	} else {
		int pos=0, len=0;
		
		QString op=text.trimmed();
		TOKEN t=Exp::getToken(op, len, tMaxOp);
		for(pos=0; pos<text.length() && text[pos].isSpace(); pos++);
		
		while(pos < text.length() && t.tipus!=tEof){
			switch(t.tipus){
				case tVal:
					if(t.val.mid(1,2)=="cn")
						setFormat(pos, len, QColor(0,0,200));
					else
						setFormat(pos, len, QColor(100,0,0));
					break;
				case tFunc:
					setFormat(pos, len, QColor(0,50,0));
					break;
				case tMaxOp:
					m_wrong = true;
					setFormat(pos, len, QColor(255,0,0));
					break;
				default:
					setFormat(pos, len, negreta);
					break;
			}
			pos += len;
			t=Exp::getToken(op, len, t.tipus);
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
			if((p=complementary(text, p))>=0)
				setFormat(p, 1, form);
		}
	}
}

int AlgebraHighlighter::complementary(const QString& t, int p)
{
	Q_ASSERT(p<t.length());
	bool opening = (t[p]=='(');
	int cat=0;
	if(p<1)
		return -1;
	do {
		if(t[p]==')')
			--cat;
		else if(t[p]=='(')
			++cat;
		
		p += opening ? 1 : -1;
	} while(p>0 && p<t.length() && cat!=0);
	
	p -= opening ? 1 : -1;
	
	if(cat!=0)
		return -2;
	return p;
}
