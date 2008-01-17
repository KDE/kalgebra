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

#include "variablesmodel.h"
#include "variables.h"
#include <KLocale>
#include <KApplication>

VariablesModel::VariablesModel(QObject *parent)
	: QAbstractTableModel(parent), m_vars(0)
{}

QVariant VariablesModel::data(const QModelIndex & index, int role) const
{
	QVariant ret;
	if(role==Qt::DisplayRole) {
		int var=index.row();
		QString key=m_vars->keys()[var];
		switch(index.column()) {
			case 0:
				ret=key;
				break;
			case 1:
				ret=m_vars->value(key)->toString();
				break;
		}
	}
	return ret;
}

QVariant VariablesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant ret;
	if(role==Qt::DisplayRole && orientation==Qt::Horizontal) {
		switch(section) {
			case 0:
				ret=i18nc("@title:column", "Name");
				break;
			case 1:
				ret=i18nc("@title:column", "Value");
				break;
		}
	}
	return ret;
}

int VariablesModel::rowCount(const QModelIndex &idx) const
{
	if(idx.isValid())
		return 0;
	else
		return m_vars->count();
}

void VariablesModel::updateInformation()
{
	reset();
}


#include "variablesmodel.moc"
