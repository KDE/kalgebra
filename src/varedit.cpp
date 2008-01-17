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
#include "variables.h"
#include "expressionedit.h"

#include <KLocale>

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>

VarEdit::VarEdit(QWidget *parent, bool modal) :
	KDialog(parent), vars(NULL), m_correct(false), m_var("x")
{
	QWidget *widget = new QWidget( this );
	this->setCaption(i18n("Add/Edit a variable"));
	this->setModal(modal);
	this->setMinimumSize(200, 200);
	this->setButtons(KDialog::Ok | KDialog::Cancel);
	this->enableButtonApply( false );
	this->setMainWidget( widget );
	

	connect( this, SIGNAL( applyClicked() ), this, SLOT( accept() ) );
	connect( this, SIGNAL( okClicked() ), this, SLOT( reject() ) );
	
	QVBoxLayout *topLayout = new QVBoxLayout(widget);
	
	m_exp = new ExpressionEdit(widget);
	connect(m_exp, SIGNAL(textChanged()), this, SLOT(edit()));
	connect(m_exp, SIGNAL(returnPressed()), this, SLOT(ok()));
	
	m_valid = new QLabel(widget);
	
	QGroupBox *buttons = new QGroupBox(i18n("Mode"));
	
	m_opt_exp  = new QRadioButton(i18n("Save the expression"));
	m_opt_calc = new QRadioButton(i18n("Calculate the expression"));
	m_opt_calc->setChecked(true);
	
	QVBoxLayout *vbox = new QVBoxLayout(buttons);
	vbox->addWidget(m_opt_calc);
	vbox->addWidget(m_opt_exp);
	buttons->setLayout(vbox);
	
	topLayout->addWidget(m_exp);
	topLayout->addWidget(m_valid);
	topLayout->addWidget(buttons);
	
	m_exp->setFocus();
}

VarEdit::~VarEdit(){}

void VarEdit::setVar(const QString& newVar)
{
	m_var=newVar;
	this->setWindowTitle(i18n("Edit '%1' value", newVar));
	if(vars==NULL)
		m_exp->setText("not available");
	else {
		if(m_exp->isMathML())
			m_exp->setText(vars->value(newVar)->toMathML());
		else
			m_exp->setText(vars->value(newVar)->toString());
	}
}

Expression VarEdit::val()
{
	
	Expression e(m_exp->text(), m_exp->isMathML());
	
	
	if(m_opt_calc->isChecked()) {
		Analitza a;
		a.setExpression(e);
		return Expression(a.calculate());
	} else
		return e;
}

void VarEdit::edit()
{
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
			m_valid->setText(i18n("%1:=%2", m_var, m_exp->text()));
		else {
			Expression val = a.calculate();
			
			if(!a.isCorrect()) {
				m_valid->setText(i18n("<b style='color:red'>WRONG</b>"));
				m_valid->setToolTip(a.m_err.join("\n"));
				m_correct=false;
			} else {
				m_valid->setText(i18n("<b style='color:#090'>%1 := %2</b>", m_var, val.toString()));
				m_valid->setToolTip(QString());
				m_correct=true;
			}
		}
	}
	
	m_exp->setCorrect(m_correct);
	enableButtonApply(m_correct);
}

void VarEdit::ok(){
	if(m_correct)
		accept();
}

QString VarEdit::text() const
{
	return m_exp->text();
}

void VarEdit::setAnalitza(Analitza * na)
{
	vars= na->m_vars;
	m_exp->setAnalitza(na);
}

#include "varedit.moc"

