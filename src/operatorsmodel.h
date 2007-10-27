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

#ifndef OPERATORSMODEL_H
#define OPERATORSMODEL_H

#include <QAbstractTableModel>
#include "operator.h"

class Variables;

/** Operators model is a model class that has a relation of all operators string with their OperatorType. */
class OperatorsModel : public QAbstractTableModel
{
	public:
		/** Constructor. Creates a new Operator Model. */
		explicit OperatorsModel(QObject *parent=0);
		
		/** Returns how many operators we have. */
	// 	int count() const { return m_count; }
		
		/** Returns the description of the @p o operator. */
		static QString description(Operator o);
		
		/** Returns the description of the @p o operator. */
		static QString example(Operator o);
		
		/** Adds an entry to the model. */
	// 	void addEntry(int i, const QString&, const QString&, const QString& ex=QString());
		
		/** Updates the variables information */
		void updateInformation();
		
		QVariant data( const QModelIndex &index, int role=Qt::DisplayRole) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
		int rowCount(const QModelIndex &parent=QModelIndex()) const;
		int columnCount(const QModelIndex &parent=QModelIndex()) const;
		
		void setVariables(const Variables* v) { m_vars=v; }
	
	private:
		const Variables *m_vars;
};

#endif
