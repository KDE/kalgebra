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

#include <QSyntaxHighlighter>
#include "expressionedit.h"
#include "exp.h"

#ifndef ALGEBRAHIGHLIGHTER_H
#define ALGEBRAHIGHLIGHTER_H

/**
 *	The AlgebraHighlighter class is used to highlight the ExpressionEdit text.
 *	@author <aleixpol@gmail.com>
 */

class AlgebraHighlighter : public QSyntaxHighlighter
{
	public:
		/** Defines the format status that could be used. */
		typedef enum {
			Disabled,	/**< No highlighting. */
			Expression,	/**< String expression format. */
			MathML,		/**< MathML format. */
			Autodetect	/**< Try to guess which format is being used. */
		} Mode;
		
		/** Constructor. Creates an AlgebraHighlighter from a QTextDocument @p doc. */
		AlgebraHighlighter(QTextDocument *doc);
		//int highlightParagraph(const QString &text, int endStateOfLastPara);
		
		/** Returns the currently highlight mode. */
		Mode mode() { return m_mode; }
		
		/** Sets the highlight mode. */
		void setMode(const Mode& newMode){ m_mode=newMode; rehighlight(); }
		
		/** 
		 *	Returns whether something wrong has been found. It is an uncomplete way
		 *	to know if it is correct because doesn't do any recursive check, but could be useful.
		 */
		bool isWrong() const { return m_wrong; }
		
		/** Sets the cursor position. */
		void setPos(int p) { m_pos=p; }
	private:
		void highlightBlock(const QString &text);
		
		typedef enum { //For mathml highlighting
			gt,	
			lt,
			tag,
			value
		} MMLtokEnum;
		
// 		TOKEN getToken(QString &a, int &l);
		//TOKEN getTokenMML(QString &a, unsigned int &l);
		bool m_wrong;
		tokEnum antnum;
		Mode m_mode;
		int m_pos;
		
		QTextCharFormat negreta;
		int complementary(const QString&, int p);
};

#endif
