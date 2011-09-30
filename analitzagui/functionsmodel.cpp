/*************************************************************************************
 *  Copyright (C) 2007-2008 by Aleix Pol <aleixpol@kde.org>                          *
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

#include "functionsmodel.h"
#include <KLocale>
#include <KDebug>
#include <QPixmap>
#include <QFont>
#include <analitza/expression.h>
#include <QIcon>

FunctionsModel::FunctionsModel(QObject *parent)
	: QAbstractTableModel(parent), m_resolution(500), m_fcount(1)
{
	QHash<int, QByteArray> rolenames=QAbstractTableModel::roleNames();
	rolenames.insert(Color, "color");
	rolenames.insert(Expression, "expression");
	rolenames.insert(Shown, "shown");
	
	setRoleNames(rolenames);
}

QVariant FunctionsModel::data(const QModelIndex & index, int role) const
{
	QVariant ret;
	if(!index.isValid() || index.row()>=funclist.count())
		return QVariant();
	int var=index.row();
	const Function &f=funclist[var];
	
	switch(role) {
		case Qt::DisplayRole:
			switch(index.column()) {
				case 0:
					ret=f.name();
					break;
				case 1:
					ret=f.expression().toString();
					break;
			}
			break;
		case Qt::DecorationRole:
			if(index.column()==0) {
				QPixmap ico(15, 15);
				ico.fill(f.color());
				ret = QIcon(ico);
			} else {
				ret = QIcon::fromTheme(f.icon());
			}
			break;
		case Expression:
			ret=f.expression().toString();
			break;
		case Shown:
			ret=f.isShown();
			break;
		case Color:
			ret=f.color();
			break;
		
	}
	
// 	qDebug() << "laaaa" << roleNames().value(role) << ret << index;
	return ret;
}

QVariant FunctionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant ret;
	if(role==Qt::DisplayRole && orientation==Qt::Horizontal) {
		switch(section) {
			case 0:
				ret=i18nc("@title:column", "Name");
				break;
			case 1:
				ret=i18nc("@title:column", "Function");
				break;
		}
	}
	return ret;
}

int FunctionsModel::rowCount(const QModelIndex &idx) const
{
	if(idx.isValid())
		return 0;
	else
		return funclist.count();
}

//TODO: really need to return that?
bool FunctionsModel::addFunction(const Function& func)
{
	Q_ASSERT(func.isCorrect());
	
	QList<Function>::const_iterator it=const_cast<const FunctionsModel*>(this)->findFunction(func.name());
	bool exists=it!=funclist.constEnd();
	
	if(!exists) {
		beginInsertRows (QModelIndex(), rowCount(), rowCount());
		funclist.append(func);
		funclist.last().setResolution(m_resolution);
		endInsertRows();
		sendStatus(i18n("%1 function added", func.name()));
		
		emit functionModified(func.name(), func.expression());
	}
	
	return !exists;
}

bool FunctionsModel::removeRows(int row, int count, const QModelIndex & parent)
{
	Q_ASSERT(row+count-1<funclist.count());
	if(parent.isValid())
		return false;
	beginRemoveRows(parent, row, row+count-1);
	
	QList<Function>::iterator it=funclist.begin()+row;
	for(int i=count-1; i>=0; i--) {
		QString name=it->name();
		it=funclist.erase(it);
		emit functionRemoved(name);
	}
	endRemoveRows();
	
	return true;
}

void FunctionsModel::clear()
{
	if(!funclist.isEmpty()) {
		beginRemoveRows (QModelIndex(), 0, rowCount());
		funclist.clear();
		endRemoveRows ();
		reset();
	}
}

bool FunctionsModel::setShown(const QString& f, bool shown)
{
	for (QList<Function>::iterator it = funclist.begin(); it != funclist.end(); ++it ){
		if(it->name() == f) {
			it->setShown(shown);
			return true;
		}
	}
	return false;
}

//TODO remove me
Function* FunctionsModel::editFunction(int num)
{
	Q_ASSERT(num<funclist.count());
	return &funclist[num];
}

void FunctionsModel::editFunction(int num, const Function& func)
{
	Q_ASSERT(num<funclist.count());
	funclist[num]=func;
	funclist[num].setResolution(m_resolution);
	
	QModelIndex idx=index(num, 0), idxEnd=index(num, columnCount()-1);
	emit dataChanged(idx, idxEnd);
	emit functionModified(func.name(), func.expression());
	
// 	this->repaint(); emit update
}

bool FunctionsModel::editFunction(const QString& toChange, const Function& func)
{
	bool exist=false;
	
	QList<Function>::iterator it=findFunction(toChange);
	if(it!=funclist.end()) {
		exist=true;
		*it = func;
		it->setName(toChange);
		it->setResolution(m_resolution);
		
		int i = funclist.indexOf(*it);
		
		QModelIndex idx=index(i, 0), idxEnd=index(i, columnCount()-1);
		emit dataChanged(idx, idxEnd);
		emit functionModified(toChange, func.expression());
	}
	
	return exist;
}

bool FunctionsModel::setData(const QModelIndex & idx, const QVariant &value, int role)
{
	if(role==Shown) {
		bool isshown=value.toBool();
		funclist[idx.row()].setShown(isshown);
		
		QModelIndex idx1=index(idx.row(), 0), idxEnd=index(idx.row(), columnCount()-1);
		emit dataChanged(idx1, idxEnd);
	}
	return false;
}

void FunctionsModel::updatePoints(int i, const QRect & viewport)
{
	Q_ASSERT(i<funclist.count());
	funclist[i].update_points(viewport);
}

QLineF FunctionsModel::slope(int row, const QPointF & dp) const
{
	QLineF ret;
	if(row<0) return ret;
	
	const Function & f = funclist[row];
	if(f.isShown()) {
		ret = f.derivative(dp);
	}
	return ret;
}

QPair<QPointF, QString> FunctionsModel::calcImage(int row, const QPointF & ndp)
{
	QPair<QPointF, QString> ret;
	if(row<0) return ret;
	
	ret.first=ndp;
	Function & f = funclist[row];
	if(f.isShown()) {
		ret = f.calc(ndp);
	}
	return ret;
}

Qt::ItemFlags FunctionsModel::flags(const QModelIndex &idx) const
{
	if(idx.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsTristate;
	else
		return 0;
}

void FunctionsModel::setResolution(uint res)
{
	m_resolution=res;
	if(!funclist.isEmpty()) {
		for (QList<Function>::iterator it=funclist.begin(); it!=funclist.end(); ++it)
			it->setResolution(res);
		QModelIndex idx=index(0, 0), idxEnd=index(rowCount()-1, 0);
		emit dataChanged(idx, idxEnd);
	}
}

QString FunctionsModel::freeId()
{
	return QString("f%1").arg(m_fcount++);
}

QList<Function>::const_iterator FunctionsModel::findFunction(const QString& id) const
{
	for (QList<Function>::const_iterator it = funclist.constBegin(); it!=funclist.constEnd(); ++it)
		if(it->name() == id)
			return it;
	
	return funclist.constEnd();
}

QList<Function>::iterator FunctionsModel::findFunction(const QString& id)
{
	for (QList<Function>::iterator it = funclist.begin(); it!=funclist.end(); ++it)
		if(it->name() == id)
			return it;
	
	return funclist.end();
}

QModelIndex FunctionsModel::indexForId(const QString& id)
{
	int i=0;
	for (QList<Function>::iterator it = funclist.begin(); it!=funclist.end(); ++it, ++i)
		if(it->name() == id)
			return index(i,0);
	return QModelIndex();
}

#include "functionsmodel.moc"
