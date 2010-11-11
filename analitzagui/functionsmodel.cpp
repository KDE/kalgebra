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
#include <KApplication>
#include <KDebug>
#include <QPixmap>
#include <QFont>
#include <analitza/expression.h>
#include <QIcon>

FunctionsModel::FunctionsModel(QObject *parent)
	: QAbstractTableModel(parent), m_selectedRow(-1), m_resolution(500), m_fcount(1)
{}

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
				ret=ico;
			} else {
				ret = QIcon::fromTheme(f.icon());
			}
			break;
		case Qt::FontRole:
			if(var==m_selectedRow) {
				QFont f(qApp->font());
				f.setBold(true);
				ret=f;
			}
			break;
		case Selection:
			ret = m_selectedRow;
			break;
		case Shown:
			ret=funclist[index.row()].isShown();
			break;
		
	}
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
	bool exists=false;
	
	for (QList<Function>::iterator it = funclist.begin(); !exists && it!=funclist.end(); ++it)
		exists = (it->name() == func.name());
	
	if(!exists) {
		beginInsertRows (QModelIndex(), rowCount(), rowCount());
		funclist.append(func);
		funclist.last().setResolution(m_resolution);
		m_selectedRow=funclist.count()-1;
		endInsertRows();
		sendStatus(i18n("%1 function added", func.name()));
		
		emit functionModified(func.name(), func.expression());
	}
	
	return exists;
}

bool FunctionsModel::removeRows(int row, int count, const QModelIndex & parent)
{
	Q_ASSERT(row+count-1<funclist.count());
	if(parent.isValid())
		return false;
	beginRemoveRows(parent, row, row+count-1);
	
	if(m_selectedRow>=row)
		m_selectedRow-=count;
	
	QList<Function>::iterator it=funclist.begin()+row;
	for(int i=count-1; i>=0; i--) {
		QString name=it->name();
		it=funclist.erase(it);
		emit functionRemoved(name);
	}
	endRemoveRows();
	
	Q_ASSERT(m_selectedRow>=-1 && m_selectedRow<funclist.count());
	
	return true;
}

void FunctionsModel::setSelected(const QModelIndex & indexSel)
{
	int previous=m_selectedRow;
	m_selectedRow=indexSel.row();
	if(previous!=m_selectedRow) {
		QModelIndex idx=index(m_selectedRow, 0), idxEnd=index(m_selectedRow, columnCount()-1);
		emit dataChanged(idx, idxEnd);
		
		idx=index(previous, 0), idxEnd=index(previous, columnCount()-1);
		emit dataChanged(idx, idxEnd);
	}
}

bool FunctionsModel::setSelected(const QString& exp)
{
	int i=0;
	int previous=m_selectedRow;
	bool found=false;
	foreach(const Function& f, funclist) {
		if(f.name() == exp) {
			m_selectedRow=i;
			found=true;
		}
		i++;
	}
	
	if(found && previous!=m_selectedRow) {
		QModelIndex idx=index(m_selectedRow, 0), idxEnd=index(m_selectedRow, columnCount()-1);
		emit dataChanged(idx, idxEnd);
		
		idx=index(previous, 0);
		idxEnd=index(previous, columnCount()-1);
		emit dataChanged(idx, idxEnd);
	}
	return found;
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
	
	int i=0;
	for (QList<Function>::iterator it = funclist.begin(); !exist && it != funclist.end(); ++it, ++i ) {
		if(it->name() == toChange) {
			exist=true;
			*it = func;
			it->setName(toChange);
			it->setResolution(m_resolution);
			QModelIndex idx=index(i, 0), idxEnd=index(i, columnCount()-1);
			emit dataChanged(idx, idxEnd);
			emit functionModified(toChange, func.expression());
		}
	}
	
	return exist;
}

bool FunctionsModel::setData(const QModelIndex & idx, const QVariant &value, int role)
{
	if(role==Selection) setSelected(idx);
	else if(role==Shown) {
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

const Function & FunctionsModel::currentFunction() const
{
	Q_ASSERT(hasSelection());
	return funclist[m_selectedRow];
}

Function & FunctionsModel::currentFunction()
{
	Q_ASSERT(hasSelection());
	return funclist[m_selectedRow];
}

QLineF FunctionsModel::slope(const QPointF & dp) const
{
	QLineF ret;
	if(hasSelection()) {
		const Function & f = currentFunction();
		if(f.isShown()) {
			ret = f.derivative(dp);
		}
	}
	return ret;
}

QPair<QPointF, QString> FunctionsModel::calcImage(const QPointF & ndp)
{
	QPair<QPointF, QString> ret;
	ret.first=ndp;
	
	if(hasSelection()){
		Function & f = currentFunction();
		if(f.isShown()) {
			ret = f.calc(ndp);
		}
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

void FunctionsModel::unselect()
{
	m_selectedRow=-1;
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

#include "functionsmodel.moc"
