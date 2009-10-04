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

#ifndef ALGEBRAHIGHLIGHTER_H
#define ALGEBRAHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include "explexer.h"

/**
 *	The AlgebraHighlighter class is used to highlight the ExpressionEdit text.
 *	@author <aleixpol@kde.org>  
 */

class Analitza;

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
		explicit AlgebraHighlighter(QTextDocument *doc, const Analitza* na=0);
		//int highlightParagraph(const QString &text, int endStateOfLastPara);
		
		/** Returns the currently highlight mode. */
		Mode mode() { return m_mode; }
		
		/** Sets the highlight mode. */
		void setMode(const Mode& newMode){ m_mode=newMode; rehighlight(); }
		
		/** 
		 *	Returns whether something wrong has been found. It is an uncomplete way
		 *	to know if it is correct because doesn't do any recursive check, but could be useful.
		 * @returns whether it is a lexically correct expression.
		 */
		bool isCorrect() const { return m_correct; }
		
		/** Sets the cursor position. */
		void setPos(uint p) { m_pos=p; }
		
		/** Sets the corresponding Analitza class. */
		void setAnalitza(const Analitza* na) { a = na; }
		
		///@returns the name of the function that's being edited, if any
		QString editingName() const;
		
		///@returns the number of the parameter that's being edited
		int editingParameter() const;
		bool editingBounds() const;
	private:
		void highlightBlock(const QString &text);
		
		enum MMLtokEnum { //For mathml highlighting
			gt,	
			lt,
			tag,
			value
		};
		
		bool m_correct;
		int antnum;
		Mode m_mode;
		uint m_pos;
		int m_editingParameter;
		QString m_editingName;
		bool m_editingBounds;
		QString m_aName;
		
		QTextCharFormat bold;
		const Analitza* a;
};

#endif
