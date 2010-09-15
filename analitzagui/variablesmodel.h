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

#ifndef VARIABLESMODEL_H
#define VARIABLESMODEL_H

#include <QAbstractTableModel>
#include "analitzaguiexport.h"

namespace Analitza
{
class Variables;
class Expression;
}

/** Variables model is a model class that has a relation of all operators string with their VariableType. */
class ANALITZAGUI_EXPORT VariablesModel : public QAbstractTableModel
{
	Q_OBJECT
	public:
		/** Constructor. Creates a new Variable Model. */
		explicit VariablesModel(Analitza::Variables* v, QObject *parent=0);
		
		virtual QFlags< Qt::ItemFlag > flags(const QModelIndex& index) const;
		bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);
		QVariant data( const QModelIndex &index, int role=Qt::DisplayRole) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
		int rowCount(const QModelIndex &parent) const;
		int columnCount(const QModelIndex &) const { return 2; }
		
		void insertVariable(const QString& name, const Analitza::Expression& value);
		void setEditable(bool ed) { m_editable=ed; }
	public slots:
		/** Updates the variables information */
		void updateInformation();
		
	private:
		Analitza::Variables *m_vars;
		bool m_editable;
};

#endif
