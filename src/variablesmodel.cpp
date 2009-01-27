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
#include "expression.h"
#include <KLocale>
#include <KApplication>
#include <KDebug>

VariablesModel::VariablesModel(Variables* v, QObject *parent)
	: QAbstractTableModel(parent), m_vars(v)
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
				if(m_vars->value(key)->type()==Object::value) {
					Cn* v=static_cast<Cn*>(m_vars->value(key));
					ret=v->value();
				} else
					ret=m_vars->value(key)->toString();
				break;
		}
	}
	return ret;
}

bool VariablesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(role!=Qt::EditRole || !value.isValid())
		return false;
	
	if(index.column()==1) { //Changing values
		QString name=data(index.sibling(index.row(), 0)).toString();
		if(value.canConvert<double>())
			m_vars->modify(name, value.value<double>());
		else
			m_vars->modify(name, Expression(value.toString(), Expression::isMathML(value.toString())));
		emit dataChanged(index, index);
		return true;
	} else if(index.column()==0) {
		QString name=data(index.sibling(index.row(), 0)).toString();
		m_vars->rename(name, value.toString());
		emit dataChanged(index, index);
		return true;
	}
	return false;
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

QFlags< Qt::ItemFlag > VariablesModel::flags(const QModelIndex& index) const
{
	QFlags< Qt::ItemFlag > ret = QAbstractItemModel::flags(index);
	if(index.column()==1)
		ret |= Qt::ItemIsEditable;
	return ret;
}


void VariablesModel::updateInformation()
{
	reset();
}

#include "variablesmodel.moc"
