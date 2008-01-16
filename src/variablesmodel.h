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

#ifndef VARIABLESMODEL_H
#define VARIABLESMODEL_H

#include <QAbstractTableModel>
#include "operator.h"

class Variables;

/** Variables model is a model class that has a relation of all operators string with their VariableType. */
class VariablesModel : public QAbstractTableModel
{
	Q_OBJECT
	public:
		/** Constructor. Creates a new Variable Model. */
		explicit VariablesModel(QObject *parent=0);
		
		QVariant data( const QModelIndex &index, int role=Qt::DisplayRole) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
		int rowCount(const QModelIndex &parent=QModelIndex()) const;
		int columnCount(const QModelIndex &p=QModelIndex()) const { Q_UNUSED(p); return 2; }
		
		void setVariables(const Variables* v) { m_vars=v; }
		const QSet<QString>& functions() const;
	public slots:
		/** Updates the variables information */
		void updateInformation();
	private:
		const Variables *m_vars;
};

#endif
