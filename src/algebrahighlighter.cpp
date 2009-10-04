
/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@kde.org>                               *
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
#include <QStack>

AlgebraHighlighter::AlgebraHighlighter(QTextDocument *doc, const Analitza *na)
	: QSyntaxHighlighter(doc), m_correct(true), m_mode(Autodetect), m_pos(0), a(na)
{
	bold.setFontWeight(QFont::Bold);
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

struct FuncItem
{
	FuncItem() : parameter(-1), pos(-1), bounding(false) {}
	FuncItem(const QString& name, int p) : id(name), parameter(p), pos(-1), bounding(false) {}
	QString id;
	int parameter;
	int pos;
	bool bounding;
};

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
	m_editingParameter=0;
	m_editingName.clear();
	
	if(Expression::isMathML(text)) {
		QString lasttag;
		int inside=0;
		for(int i=0; i<text.length(); i++) {
			if(text[i]=='<') { //We enter in a tag
				lasttag.clear();
				int j=i+1, k=0;
				for(k=i+1; k<text.length() && text[k]!='>'; ++k){
					lasttag.append(text[k]);
					if(text[k]!=' ' && j==k-1)
						j=k;
				}
				j++;
				
				setFormat(i, 1, bold);
				setFormat(j, 1, bold);
				if(lasttag.startsWith(QChar('/'))){
					setFormat(i+1, j-i-1, QColor(100,0,0));
					setFormat(i+1, 1, bold);
					inside--;
				} else if(lasttag.endsWith(QChar('/'))) {
					setFormat(i+1, j-i-1, id);
					setFormat(j+1, 2, bold);
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
		QStack<ExpLexer::TOKEN> paren;
		QStack<FuncItem> parameter;
		
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
					m_aName=lex.current.val;
					break;
				case -1:
					m_correct = false;
					f=uncorrect;
					break;
				case ExpressionTable::tLpr:
				case ExpressionTable::tLcb:
					paren.push(lex.current);
					if(m_pos>lex.current.pos)
						parameter.push(FuncItem(m_aName, 0));
					isBold=true;
					setFormat(lex.current.pos, lex.current.len, bold);
					break;
				case ExpressionTable::tRpr:
				case ExpressionTable::tRcb: {
					isBold=true;
					setFormat(lex.current.pos, lex.current.len, bold);
					
					bool corr=!paren.isEmpty();
					if(corr) {
						if(lex.current.type==ExpressionTable::tRpr && paren.top().type!=ExpressionTable::tLpr)
							corr=false;
						
						if(m_pos==lex.current.pos || m_pos==paren.top().pos) {
							QTextCharFormat bg=bold;
							bg.setBackground(Qt::yellow);
							
							setFormat(lex.current.pos, lex.current.len, bg);
							setFormat(paren.top().pos, paren.top().len, bg);
						}
						
						paren.pop();
						if(m_pos>lex.current.pos)
							parameter.pop();
					}
					
					if(!corr)
						setFormat(lex.current.pos, lex.current.len, uncorrect); // error
				}	break;
				default:
					if(m_pos>lex.current.pos && !parameter.isEmpty()
							&& lex.current.type==ExpressionTable::tComa) {
						parameter.top().parameter++;
						parameter.top().pos=lex.current.pos;
					}
					
					if(m_pos>lex.current.pos && !parameter.isEmpty()
							&& lex.current.type==ExpressionTable::tColon) {
						parameter.top().bounding=true;
						parameter.top().pos=lex.current.pos;
					}
					
					isBold=true;
					setFormat(lex.current.pos, lex.current.len, bold);
					break;
			}
			if(!isBold)
				setFormat(lex.current.pos, lex.current.len, f);
		}
		
		if(!lex.error().isEmpty())
		{
			setFormat(lex.current.pos, text.size()-lex.current.pos, uncorrect);
		}
		
		if(!parameter.isEmpty()) {
			m_editingName=parameter.top().id;
			m_editingParameter=parameter.top().parameter;
			m_editingBounds=parameter.top().bounding;
			
			if(m_editingParameter!=0 || m_editingBounds) {
				QTextCharFormat currentComa=bold;
				currentComa.setBackground(Qt::yellow);
				setFormat(parameter.top().pos, 1, currentComa);
			}
		}
	}
}

QString AlgebraHighlighter::editingName() const
{
	return m_editingName;
}

int AlgebraHighlighter::editingParameter() const
{
	return m_editingParameter;
}

bool AlgebraHighlighter::editingBounds() const
{
	return m_editingBounds;
}
