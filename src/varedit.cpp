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

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <klocalizedstring.h>

VarEdit::VarEdit(QWidget *parent, bool modal)
    : QDialog(parent), vars(0), m_correct(false)
{
    setWindowTitle(i18n("Add/Edit a variable"));
    setModal(modal);

    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_removeBtn = m_buttonBox->addButton(i18n("Remove Variable"), QDialogButtonBox::DestructiveRole);
    m_removeBtn->setIcon(QIcon::fromTheme("edit-table-delete-row"));
    
    connect(m_removeBtn, SIGNAL(clicked(bool)), SLOT(removeVariable()));
    
    connect( this, SIGNAL(applyClicked()), this, SLOT(accept()) );
    connect( this, SIGNAL(okClicked()), this, SLOT(reject()) );
    
    m_exp = new Analitza::ExpressionEdit(this);
    connect(m_exp, SIGNAL(textChanged()), this, SLOT(edit()));
    connect(m_exp, SIGNAL(returnPressed()), this, SLOT(ok()));
    
    m_valid = new QLabel(this);
    
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->addWidget(m_exp);
    topLayout->addWidget(m_valid);
    topLayout->addWidget(m_buttonBox);
    
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
    
    m_removeBtn->setEnabled(canRemove(newVar));
}

bool VarEdit::canRemove(const QString& name) const
{
    QHash<QString, Analitza::Object*>::const_iterator it = vars->constBegin(), itEnd = vars->constEnd();
    for(; it!=itEnd; ++it) {
        QStringList deps = AnalitzaUtils::dependencies(*it, QStringList());
        if(deps.contains(name)) {
            return false;
        }
    }
    return true;
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
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_correct);
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

void VarEdit::removeVariable()
{
    vars->remove(m_var);
    close();
}
