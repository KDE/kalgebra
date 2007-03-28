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

#include "varedit.h"
#include "analitza.h"
#include "expression.h"

#include <klocale.h>

#include <QPushButton>
#include <QLabel>

VarEdit::VarEdit(QWidget *parent, bool modal) :
	QDialog (parent), vars(NULL), m_correct(false), m_var("x")
{
	this->setWindowTitle(i18n("Add/Edit a variable"));
	this->setModal(modal);
	this->setMinimumSize(200, 200);
	
	buttonBox = new QDialogButtonBox(this);
	buttonBox->setOrientation(Qt::Horizontal);
	buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
	QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	QVBoxLayout *topLayout = new QVBoxLayout(this);
	
	m_exp = new ExpressionEdit(this);
	connect(m_exp, SIGNAL(textChanged()), this, SLOT(edit()));
	connect(m_exp, SIGNAL(returnPressed()), this, SLOT(ok()));
	
	m_valid = new QLabel(this);
	
	QGroupBox *buttons = new QGroupBox(i18n("Mode"));
	
	m_opt_exp  = new QRadioButton(i18n("Save the expression"));
	m_opt_calc = new QRadioButton(i18n("Calculate the expression"));
	m_opt_calc->setChecked(true);
	
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(m_opt_calc);
	vbox->addWidget(m_opt_exp);
	buttons->setLayout(vbox);
	
	topLayout->addWidget(m_exp);
	topLayout->addWidget(m_valid);
	topLayout->addWidget(buttons);
	topLayout->addWidget(buttonBox);
	
	m_exp->setFocus();
}

VarEdit::~VarEdit(){}

void VarEdit::setVar(const QString& newVar)
{
	m_var=newVar;
	this->setWindowTitle(i18n("Edit '%1' value").arg(newVar));
	if(vars==NULL)
		m_exp->setText("not available");
	else {
		if(m_exp->isMathML())
			m_exp->setText(vars->value(newVar)->toMathML());
		else
			m_exp->setText(vars->value(newVar)->toString());
	}
}

Object* VarEdit::val()
{
	Analitza a;
	Object* ret;
	
	Expression e;
	if(m_exp->isMathML())
		e.setMathML(m_exp->text());
	else
		e.setText(m_exp->text());
	
	if(m_opt_calc->isChecked())
		ret = new Cn(a.calculate());
	else
		ret = Expression::objectCopy(a.expression()->m_tree);
	
	return ret;
}

void VarEdit::edit()
{
	double val;
	Analitza a;
	QString funct = m_exp->text();
	
	if(m_exp->text().isEmpty()) {
		m_exp->setCorrect(true);
		m_valid->setText(QString());
		return;
	}
	
	Expression e;
	if(m_exp->isMathML())
		e.setMathML(m_exp->text());
	else
		e.setText(m_exp->text());
	
	if(!e.isCorrect()) {
		a.m_err << i18n("From parser:") << e.error();
		return;
	} else
		a.setExpression(e);
	
	if(a.isCorrect()) {
		if(a.bvarList().count()>0)
			m_valid->setText(i18n("%1:=%2").arg(m_var).arg(m_exp->text()));
		else {
			val = a.calculate().value();
			
			if(!a.isCorrect()) {
				m_valid->setText(i18n("<b style='color:red'>WRONG</b>"));
				m_valid->setToolTip(a.m_err.join("\n"));
				m_correct=false;
			} else {
				m_valid->setText(i18n("<b style='color:#090'>%1 := %2</b>").arg(m_var).arg(val));
				m_valid->setToolTip(QString());
				m_correct=true;
			}
		}
	}
	
	m_exp->setCorrect(m_correct);
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_correct);
}

void VarEdit::ok(){
	if(m_correct)
		accept();
}

#include "varedit.moc"

