/*************************************************************************************
 *  Copyright (C) 2010 by Aleix Pol <aleixpol@kde.org>                               *
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


#ifndef FUNCTIONSDIALOG_H
#define FUNCTIONSDIALOG_H

#include <QDialog>
#include <QTreeView>

namespace Analitza {
class Variables;
}

class FunctionEdit;
class FunctionsModel;
class FunctionsDialog : public QDialog
{
	Q_OBJECT
	public:
		explicit FunctionsDialog(FunctionsModel* model, QWidget* parent = 0);
		
	public slots:
		void addFunction();
		void addEditedFunction();
		
	private:
		QTreeView* m_view;
		FunctionsModel* m_model;
		
		FunctionEdit* m_edit;
		QDialog* m_editDialog;
		Analitza::Variables* m_vars;
		
};

#endif // FUNCTIONSDIALOG_H
