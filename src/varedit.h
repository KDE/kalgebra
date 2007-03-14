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

#ifndef VAREDIT_H
#define VAREDIT_H

#include <QToolTip>
#include <QDialog>
#include <QLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLabel>

#include "expressionedit.h"

/**
@author Aleix Pol i Gonzalez
*/

class VarEdit : public QDialog
{
Q_OBJECT
public:
	explicit VarEdit(QWidget *parent = 0, bool modal = false);
	~VarEdit();
	QString text() const { return m_exp->text(); }
	void setVar(const QString& newVar);	//This should edit the variable name
	void setMode(int mode);			//This should select what Option we need
	void setAnalitza(Analitza *na) { vars= na->m_vars; m_exp->setAnalitza(na); }
	Object* val();
private:
	ExpressionEdit *m_exp;
	
	QRadioButton *m_opt_calc;		//Per escollir guardar calcul
	QRadioButton *m_opt_exp;		//Per escollir guardar expressio
	
	QLabel *m_valid;
	Variables *vars;
	bool m_correct;
	QString m_var;
	
	QDialogButtonBox *buttonBox;
private slots:
	void edit();
	void ok();
};

#endif
