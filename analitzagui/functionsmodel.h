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

#ifndef FUNCTIONSMODEL_H
#define FUNCTIONSMODEL_H

#include <QAbstractTableModel>
#include "analitzagui/function.h"

#include "analitzaguiexport.h"

/** Functions model is a model class that has a relation of all operators string with their FunctionType. */
class ANALITZAGUI_EXPORT FunctionsModel : public QAbstractTableModel
{
	Q_OBJECT
	Q_PROPERTY(uint resolution READ resolution WRITE setResolution);
	public:
		enum FunctionsModelRoles { Shown=Qt::UserRole+2 };
		typedef QList<Function>::const_iterator const_iterator;
		friend class Graph2D;
		
		/** Constructor. Creates a new Function Model. */
		explicit FunctionsModel(QObject *parent=0);
		
		Qt::ItemFlags flags ( const QModelIndex & index ) const;
		
		QVariant data( const QModelIndex &index, int role=Qt::DisplayRole) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const;
		int rowCount(const QModelIndex &parent=QModelIndex()) const;
		int columnCount(const QModelIndex & =QModelIndex()) const { return 2; }
		
		/** Adds another function @p f. Returns whether another function like @p f existed. */
		bool addFunction(const Function& f);
	
		/** Specifies that the @p exp function is shown.
			@returns whether another function like @p exp existed. */
		bool setShown(const QString& exp, bool shown);
	
		/** Edits the @p num nth function. The @p num should be less than the number of functions,
			because you are editing. */
		void editFunction(int num, const Function& func);
	
		/** Edits the @p exp function. Returns whether another function like @p exp existed. */
		bool editFunction(const QString& name, const Function& func);
	
		/** Returns a pointer to the @p num nth function. */
		Function* editFunction(int num);
		
		void setResolution(uint res);
		uint resolution() const { return m_resolution; }
		
		void clear();
		
		void sendStatus(const QString& msg) { emit status(msg); }
		
		void updatePoints(int i, const QRect& viewport);
		
		const_iterator constBegin() const { return funclist.constBegin(); }
		const_iterator constEnd() const { return funclist.constEnd(); }
		
		virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
		
		virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
		
		/** Returns the id for the next function that's not used by any other, starting by f */
		QString freeId();
		
		QPair<QPointF, QString> calcImage(int row, const QPointF& ndp);
		QLineF slope(int row, const QPointF& dp) const;
		QModelIndex indexForId(const QString & name);
		
	signals:
		/** Emits a status message when something changes. */
		void status(const QString &msg);
		
		void functionModified(const QString& name, const Analitza::Expression& e);
		void functionRemoved(const QString& name);
		
	private:
		QList<Function>::const_iterator findFunction(const QString& id) const;
		QList<Function>::iterator findFunction(const QString& id);
		
		QList<Function> funclist;
		uint m_resolution;
		
		uint m_fcount;
};

#endif
