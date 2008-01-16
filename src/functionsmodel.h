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

#ifndef FUNCTIONSMODEL_H
#define FUNCTIONSMODEL_H

#include <QAbstractTableModel>
#include "function.h"

/** Functions model is a model class that has a relation of all operators string with their FunctionType. */
class FunctionsModel : public QAbstractTableModel
{
	Q_OBJECT
	public:
		enum FunctionsModelRoles { Selection=Qt::UserRole+1, Shown=Qt::UserRole+2 };
		typedef QList<function>::const_iterator const_iterator;
		friend class Graph2D;
		
		/** Constructor. Creates a new Function Model. */
		explicit FunctionsModel(QObject *parent=0);
		
		Qt::ItemFlags flags ( const QModelIndex & index ) const;
		
		QVariant data( const QModelIndex &index, int role=Qt::DisplayRole) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
		int rowCount(const QModelIndex &parent=QModelIndex()) const;
		int columnCount(const QModelIndex & =QModelIndex()) const { return 2; }
		
		/** Adds another function @p f. Returns whether another function like @p f existed. */
		bool addFunction(const function& f);
	
		/** Sets the function selected to @p exp. Returns whether another function like @p exp existed. */
		bool setSelected(const QString& name);
		
		inline bool isSelected(int i) const { return i==m_selectedRow; }
	
		/** Specifies that the @p exp function is shown. Returns whether another function like @p exp existed. */
		bool setShown(const QString& exp, bool shown);
	
		/** Edits the @p num nth function. The @p num should be less than the number of functions, because you are editing. */
		void editFunction(int num, const function& func);
	
		/** Edits the @p exp function. Returns whether another function like @p exp existed. */
		bool editFunction(const QString& name, const function& func);
	
		/** Returns a pointer to the @p num nth function. */
		function* editFunction(int num);
		
		void clear();
		
		void sendStatus(const QString& msg) { emit status(msg); }
		
		void updatePoints(int i, const QRect& viewport, int resolution);
		
		const function& currentFunction() const;
		
		function& currentFunction();
		
		bool hasSelection() const { return m_selectedRow>=0 && !funclist.isEmpty(); }
		
		const_iterator constBegin() const { return funclist.constBegin(); }
		const_iterator constEnd() const { return funclist.constEnd(); }
		
		virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
		
	public slots:
		void setSelected(const QModelIndex& idx);
		
	protected:
// 		const QList<function> & functionList() const { return funclist; }
		
		/** Returns the position in the list of the selected function */
		int selectedRow() const { return m_selectedRow; }
		
	signals:
		/** Emits a status when it changes. */
		void status(const QString &msg);
	private:
		QLineF slope(const QPointF& dp) const;
		QPair<QPointF, QString> calcImage(const QPointF& dp);
		
		QList<function> funclist;
		int m_selectedRow;
};

#endif
