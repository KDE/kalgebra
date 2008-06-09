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

#ifndef FUNCTIONEDIT_H
#define FUNCTIONEDIT_H

#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>

#include <kcolorcombo.h>

#include "expressionedit.h"

class Graph2D;

/**
 *	The color combo shows a different colors list.
 *	\deprecated
 *	@author Aleix Pol i Gonzalez
 */
#if 0
class ColorCombo : public QComboBox
{
Q_OBJECT
public:
	/** Constructor. Creates a new ColorCombo. */
	ColorCombo(QWidget* parent=0);
	
	/** Destructor. */
	~ColorCombo(){}
	
	/** Returns the selected Color. */
	QColor color() const;
	
	/** Sets a color to the selected color. If it didn't exist, it is added to the list. */
	void setColor(const QColor &);
private:
	void resizeEvent(QResizeEvent * event);
};
#endif
class FunctionsModel;

/**
 *	The FunctionEdit dialog provides a way to specify functions.
 *	@author Aleix Pol i Gonzalez
 */

class FunctionEdit : public QWidget
{
Q_OBJECT
public:
	/** Constructor. */
	explicit FunctionEdit(QWidget *parent=0, Qt::WFlags f = 0 );
	
	/** Destructor. */
	~FunctionEdit();
	
	/** Retrieves the resulting expression text. */
	QString text() const { return m_func->toPlainText(); }
	
	/** Sets an expression text to the ExpressionEdit widget. */
	void setText(const QString &newText);
	
	/** Retrieves the selected color for the function */
	QColor color() const { return m_color->color(); }
	
	/** Sets the selected color for the function.*/
	void setColor(const QColor &newColor);
	
	/** Returns whether there is a correct function. */
	bool isCorrect() const { return m_correct; }
	
	/** Returns whether it is a MathML function. */
	bool isMathML() const { return m_func->isMathML();}
	
	/** Returns whether we are editing or adding a function. */
	bool editing() const { return m_modmode; }
	
	/** Sets whether we are editing or adding a function. */
	void setEditing(bool m) { m_modmode=m; }
	
	/** Sets a name to the function. (Not used YET) */
	void setName(const QString& name) { m_name->setText(name); }
	
	/** Retrieves a name for the function. (Not used YET) */
	QString name() const { return m_name->text(); }
	
public slots:
	/** Clears the dialog. */
	void clear();
	
signals:
	/** Tells that the result has been accepted. */
	void accept();
private:
	ExpressionEdit *m_func;
	QLineEdit *m_name;
	QPushButton *m_ok;
	QLabel *m_valid;
	QLabel *m_validIcon;
	Graph2D *m_graph;
	KColorCombo *m_color;
	FunctionsModel *m_funcsModel;
	
	bool m_correct;
	bool m_modmode;
	
	void focusInEvent(QFocusEvent*);
private slots:
	void edit();
	void ok();
	void colorChange(int);
};

#endif
