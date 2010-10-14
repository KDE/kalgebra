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

#include "functionsdialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <KLocalizedString>
#include <KIcon>
#include <analitzagui/functionsmodel.h>
#include <analitzagui/functionsview.h>
#include <analitzagui/functionedit.h>
#include <analitza/variables.h>

FunctionsDialog::FunctionsDialog(FunctionsModel* model, QWidget* parent)
	: QDialog(parent), m_model(model), m_vars(new Analitza::Variables)
{
	setLayout(new QVBoxLayout);
	
	m_view = new FunctionsView(this);
	m_view->setModel(m_model);
	layout()->addWidget(m_view);
	
	QPushButton* add = new QPushButton(KIcon("list-add"), i18n("Add"), this);
	connect(add, SIGNAL(clicked(bool)), SLOT(addFunction()));
	layout()->addWidget(add);
	
	QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Close);
	layout()->addWidget(box);
	
	connect(box, SIGNAL(rejected()), this, SLOT(reject()));
	
	m_editDialog = new QDialog(this);
	m_editDialog->setModal(true);
	m_editDialog->setLayout(new QVBoxLayout);
	
	m_edit = new FunctionEdit(m_editDialog);
	m_edit->setVariables(m_vars);
	m_editDialog->layout()->addWidget(m_edit);
	connect(m_edit, SIGNAL(accept()), m_editDialog, SLOT(accept()));
	connect(m_editDialog, SIGNAL(accepted()), SLOT(addEditedFunction()));
}

void FunctionsDialog::addFunction()
{
	m_editDialog->show();
}

void FunctionsDialog::addEditedFunction()
{
	m_model->addFunction(m_edit->createFunction());
}
