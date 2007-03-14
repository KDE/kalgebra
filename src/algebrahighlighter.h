/*************************************************************************************
 *  one line to give the program's name and an idea of what it does.                 *
 *  Copyright (C) 2007  Aleix Pol                                                    *
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

#include <QSyntaxHighlighter>
#include "expressionedit.h"
#include "exp.h"

#ifndef ALGEBRAHIGHLIGHTER_H
#define ALGEBRAHIGHLIGHTER_H

class AlgebraHighlighter : public QSyntaxHighlighter
{
	public:
		typedef enum {
			Disabled,
			Expression,
			MathML,
			Autodetect
		} Mode;
		
		typedef enum { //For mathml highlighting
			gt,
			lt,
			tag,
			value
		} MMLtokEnum;
		
		AlgebraHighlighter(QTextDocument *doc);
		//int highlightParagraph(const QString &text, int endStateOfLastPara);
		Mode mode() { return m_mode; }
		void setMode(const Mode& newMode){ m_mode=newMode; rehighlight(); }
		bool isWrong() const { return wrong; }
		void setPos(int p) { m_pos=p; }
	private:
		TOKEN getToken(QString &a, int &l);
		//TOKEN getTokenMML(QString &a, unsigned int &l);
		bool wrong;
		tokEnum antnum;
		Mode m_mode;
		int m_pos;
		
		QTextCharFormat negreta;
		int complementary(const QString&, int p);
	protected:
		void highlightBlock(const QString &text);
};

#endif
