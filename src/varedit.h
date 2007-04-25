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

#include <KDialog>

#include <QToolTip>
#include <QLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLabel>

class Analitza;
class Object;
class Variables;
class ExpressionEdit;

/**
 *	The VarEdit provides a dialog to allow users to edit/create a variable.
 *	@author Aleix Pol i Gonzalez
 */

class VarEdit : public KDialog
{
	Q_OBJECT
	public:
		/** Constructor. Creates a variable editing dialog. */
		explicit VarEdit(QWidget *parent = 0, bool modal = false);
		
		/** Destructor. */
		~VarEdit();
		
		/** Returns the variable expression */
		QString text() const;
		
		/** Sets the editing variable name */
		void setVar(const QString& newVar);	//This should edit the variable name
		
		/** Sets an Analitza which will evaluate it. It may be interesting because variables can change. */
		void setAnalitza(Analitza *na);
		
		/** Returns the resulting variable expression */
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
