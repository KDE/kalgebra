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

#include "varedit.h"
#include <analitza/analyzer.h>
#include <analitza/expression.h>
#include <analitza/variables.h>
#include <analitzagui/expressionedit.h>

#include <KLocale>

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <KPushButton>

VarEdit::VarEdit(QWidget *parent, bool modal) :
	KDialog(parent), vars(0), m_correct(false)
{
	QWidget *widget = new QWidget( this );
	setCaption(i18n("Add/Edit a variable"));
	setModal(modal);
	setButtons(KDialog::Ok | KDialog::Cancel);
	enableButtonApply( false );
	setMainWidget( widget );
	
	setButtonIcon(KDialog::Ok, KIcon("dialog-ok"));
	setButtonIcon(KDialog::Cancel, KIcon("dialog-cancel"));
	
	connect( this, SIGNAL(applyClicked()), this, SLOT(accept()) );
	connect( this, SIGNAL(okClicked()), this, SLOT(reject()) );
	
	m_exp = new ExpressionEdit(widget);
	connect(m_exp, SIGNAL(textChanged()), this, SLOT(edit()));
	connect(m_exp, SIGNAL(returnPressed()), this, SLOT(ok()));
	
	m_valid = new QLabel(widget);
	
	QVBoxLayout *topLayout = new QVBoxLayout(widget);
	topLayout->addWidget(m_exp);
	topLayout->addWidget(m_valid);
	
	m_exp->setFocus();
}

void VarEdit::setName(const QString& newVar)
{
	m_var=newVar;
	setWindowTitle(i18n("Edit '%1' value", newVar));
	if(!vars)
		m_exp->setText(i18n("not available"));
	else {
		m_exp->setExpression(Analitza::Expression(vars->value(newVar)->copy()));
	}
}

Analitza::Expression VarEdit::val()
{
	Analitza::Expression val;
	Analitza::Analyzer a(vars);
	
	a.setExpression(m_exp->expression());
	
	if(a.isCorrect()) {
		a.simplify();
		val=a.expression();
	}
	
	m_correct = a.isCorrect();
	if(m_correct) {
		m_valid->setText(i18n("<b style='color:#090'>%1 := %2</b>", m_var, val.toString()));
		m_valid->setToolTip(QString());
	} else {
		m_valid->setText(i18n("<b style='color:red'>WRONG</b>"));
		m_valid->setToolTip(a.errors().join("\n"));
	}
	button(Ok)->setEnabled(m_correct);
	m_exp->setCorrect(m_correct);
	
	return val;
}

void VarEdit::edit()
{
	val();
}

void VarEdit::ok()
{
	if(m_correct)
		accept();
}

void VarEdit::setAnalitza(Analitza::Analyzer * na)
{
	vars= na->variables();
	m_exp->setAnalitza(na);
}

#include "varedit.moc"

