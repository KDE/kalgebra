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

#include "algebrahighlighter.h"
#include "analitza.h"
#include "exp.h"


/**
*	@author Aleix Pol i Gonzalez <aleixpol@gmail.com>
*/

class ExpressionEdit : public QTextEdit
{
Q_OBJECT
public:
	explicit ExpressionEdit(QWidget *parent = 0, AlgebraHighlighter::Mode ini=AlgebraHighlighter::Autodetect);
	~ExpressionEdit();
	bool mode() { return m_highlight->mode(); }
	void setMode(AlgebraHighlighter::Mode en);
	void setAutocomplete(bool a);
	bool autocomplete();
	
	bool isMathML() const;
	QString editingWord(int pos, int &param);
	void setAnalitza(Analitza* in) {a=in; updateCompleter(); }
	QString text() const { return this->toPlainText();}
	void setText(const QString &str) { setPlainText(str);}
	void setCorrect(bool cor);
	bool isCorrect() const { return m_correct; }
	void setCheck(bool check) { m_check=check; }
	bool check() const { return m_check; }
	void setAns(const QString &ans) { m_ans=ans; }
	QString ans() const { return m_ans; }
	void focusInEvent (QFocusEvent * event);
	
private:
	QLabel *m_helptip;
	AlgebraHighlighter *m_highlight;
	
	void helpShow(const QString& funcname, int param=0);
	static QString findPrec(const QString& exp, int &act, int cur, int &param, const QString& tit);
	void ajudant(const QString&, QPoint p);
	QString lastWord(int);
	
	void removenl();
	void keyPressEvent(QKeyEvent * e);
	
	int m_histPos;
	QStringList m_history;
	bool help;
	bool m_auto;
	
	Analitza *a;
	bool m_check;
	bool m_correct;
	QString m_ans;
	QCompleter *m_completer;
	//QStandardItemModel *m_words;
	
public slots:
	void returnP();
	void cursorMov();
	void ajudant(const QString&);
// 	void completed(const QModelIndex &);
	void completed(const QString &);
	void updateCompleter();
signals:
	void signalHelper(const QString&);
	void returnPressed();
};

#endif
