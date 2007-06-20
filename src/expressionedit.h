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

#ifndef EXPRESSIONEDIT_H
#define EXPRESSIONEDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QCompleter>
#include <QLabel>
#include <QStandardItemModel>
#include <QTreeView>
#include "algebrahighlighter.h"

class Analitza;

/**
*	The expression edit widget is the one where we will input our expressions.
*	
*	@author Aleix Pol i Gonzalez <aleixpol@gmail.com>
*/

class ExpressionEdit : public QTextEdit
{
	Q_OBJECT
	public:
		/** Constructor. Creates a new ExpressionEdit.
		*	@param parent is the widget parent.
		*	@param ini specifies what input format is going to expect the highlighting.
		*/
		explicit ExpressionEdit(QWidget *parent = 0, AlgebraHighlighter::Mode ini=AlgebraHighlighter::Autodetect);
		
		/** Destructor. */
		~ExpressionEdit();
		
		/** Returns the ExpressionEdit input mode. */
		AlgebraHighlighter::Mode mode() { return m_highlight->mode(); }
		
		/** Sets the ExpressionEdit input mode. */
		void setMode(AlgebraHighlighter::Mode en);
		
		/** Sets whether autocompletion will be used. */
		void setAutocomplete(bool a);
		
		/** Returns whether autocompletion will be used. */
		bool autocomplete();
		
		/** Returns whether there is MathML on the widget. */
		bool isMathML() const;
		
		/** Sets an Analitza @p in module associated to the ExpressionEdit. It is used to autocomplete variables. */
		void setAnalitza(Analitza* in);
		
		/** Returns the expression string that we have. */
		QString text() const { return this->toPlainText();}
		
		/** Sets the expression string @p str we have. */
		void setText(const QString &str) { setPlainText(str);}
		
		/** Sets whether it is correct. Changes the background color. */
		void setCorrect(bool cor);
		
		/** Checks whether it has been set to correct. */
		bool isCorrect() const { return m_correct; }
		
		/** Sets the string @p ans that will be entered when an operator is pressed. */
		void setAns(const QString &ans) { m_ans=ans; }
		
		/** Returns the string that will be entered when an operator is pressed. */
		QString ans() const { return m_ans; }
	public slots:
		
		/** Notifies that the cursor has moved. */
		void cursorMov();
		
		/** Shows a little tip widget containing the string @p str. If @p str is empty the widget is hidden. */
		void ajudant(const QString& str);
	// 	void completed(const QModelIndex &);
		
		/** Tries to complete the currently edited word with the @p word. */
		void completed(const QString &word);
		
		/** Updates the completer text list. */
		void updateCompleter();
		
		/** Simplify the current expression. */
		void simplify();
		
		/** Sets the Mode to MathML */
		void toMathML() { setMode(AlgebraHighlighter::MathML); }
		
		/** Sets the Mode to Expression */
		void toExpression() { setMode(AlgebraHighlighter::Expression); }
		
		/** Is the execution function, when return is pressed. */
		void returnP(); //FIXME: Change my name please
	signals:
		/** Emits that a return has been pressed. */
		void returnPressed();
		
		/** Deprecated. */
		void signalHelper(QString);
	protected:
		/** Inherited from QTextEdit, just deals with the menu. */
		void contextMenuEvent(QContextMenuEvent * e);
	
	private:
		
		QLabel *m_helptip;
		AlgebraHighlighter *m_highlight;
		
		void helpShow(const QString& funcname, int param=0);
		static QString findPrec(const QString& exp, int &act, int cur, int &param, const QString& tit);
		void ajudant(const QString&, const QPoint& p);
		QString lastWord(int);
		QString editingWord(int pos, int &param);
		void focusInEvent (QFocusEvent * event);
		
		void removenl();
		void keyPressEvent(QKeyEvent * e);
		
		int m_histPos;
		QStringList m_history;
		bool help;
		bool m_auto;
		
		Analitza *a;
		bool m_correct;
		QString m_ans;
		QCompleter *m_completer;
		QTreeView *treeView;
		//QStandardItemModel *m_words;
};

#endif
