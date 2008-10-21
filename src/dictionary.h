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

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <QWidget>
#include <QSortFilterProxyModel>

class QLabel;
class QModelIndex;
class Graph2D;
class FunctionsModel;
class OperatorsModel;
class KFormula;

/**
	@author Aleix Pol
*/
class Dictionary : public QWidget
{
Q_OBJECT
	public:
		Dictionary(QWidget *p=0);
		virtual ~Dictionary(){}
		
		QSortFilterProxyModel* model() const { return m_sortProxy; }
		
	public slots:
		void activated(const QModelIndex& prev, const QModelIndex& );
		void filterChanged(const QString&);
		
	private:
		QLabel *m_name;
		QLabel *m_descr;
		QLabel *m_sample;
		QLabel *m_example;
		KFormula *m_formula;
		Graph2D *m_graph;
		FunctionsModel *m_funcs;
		OperatorsModel *m_ops;
		QSortFilterProxyModel *m_sortProxy;
};

#endif
