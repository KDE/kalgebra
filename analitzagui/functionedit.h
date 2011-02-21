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

#ifndef FUNCTIONEDIT_H
#define FUNCTIONEDIT_H

#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QComboBox>

#include <KColorCombo>
#include <KLineEdit>

#include "analitzaguiexport.h"

class KTabWidget;
class Function;
class Graph2D;
class FunctionsModel;
class ExpressionEdit;
namespace Analitza
{
class Variables;
class Expression;
}

/**
 *	The FunctionEdit dialog provides a way to specify functions.
 *	@author Aleix Pol i Gonzalez
 */

class ANALITZAGUI_EXPORT FunctionEdit : public QWidget
{
Q_OBJECT
public:
	/** Constructor. */
	explicit FunctionEdit(QWidget *parent=0);
	
	/** Destructor. */
	~FunctionEdit();
	
	/** Retrieves the resulting expression text. */
	Analitza::Expression expression() const;
	
	Function createFunction() const;
	
	/** Sets an expression text to the ExpressionEdit widget. */
	void setFunction(const QString &newText);
	
	/** Retrieves the selected color for the function */
	QColor color() const { return m_color->color(); }
	
	/** Sets the selected color for the function.*/
	void setColor(const QColor &newColor);
	
	/** Returns whether it is a MathML function. */
	bool isMathML() const;
	
	/** Returns whether we are editing or adding a function. */
	bool editing() const { return m_modmode; }
	
	/** Sets whether we are editing or adding a function. */
	void setEditing(bool m) { m_modmode=m; }
	
	/** Sets a name to the function. (Not used YET) */
	void setName(const QString& name) { m_name->setText(name); }
	
	/** Retrieves a name for the function. (Not used YET) */
	QString name() const { return m_name->text(); }
	
	/** Sets the variables class to be used with the graph functions*/
	void setVariables(Analitza::Variables* v) { m_vars=v; }
	
	Analitza::Variables* variables() const { return m_vars; }
	
	void setOptionsShown(bool shown);
	
public slots:
	/** Clears the dialog. */
	void clear();
	
signals:
	/** Tells that the result has been accepted. */
	void accept();
	
private slots:
	void edit();
	void ok();
	void colorChange(int);
	void updateUplimit();
	void updateDownlimit();
	
private:
	void setState(const QString& text, bool negative);
	void focusInEvent(QFocusEvent*);
	
	ExpressionEdit *m_func;
	ExpressionEdit *m_uplimit, *m_downlimit;
	double m_calcUplimit, m_calcDownlimit;
	KLineEdit *m_name;
	QPushButton *m_ok;
	QLabel *m_valid;
	QLabel *m_validIcon;
	Graph2D *m_graph;
	KColorCombo *m_color;
	FunctionsModel *m_funcsModel;
	Analitza::Variables* m_vars;
	
	bool m_modmode;
	KTabWidget* m_viewTabs;
};

#endif
