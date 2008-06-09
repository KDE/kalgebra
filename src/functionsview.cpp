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

#include "functionsview.h"
#include "functionsmodel.h"
#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <QMouseEvent>
#include <QMenu>

FunctionsView::FunctionsView(QWidget * parent) : QTreeView(parent)
{
}

void FunctionsView::selectionChanged(const QItemSelection & selected, const QItemSelection &)
{
	if(!selected.indexes().isEmpty()) {
		QModelIndex idx=selected.indexes().first();
		model()->setData(idx, QVariant(), FunctionsModel::Selection);
	}
}

void FunctionsView::mousePressEvent(QMouseEvent * e)
{
	QModelIndex clickIdx(indexAt(e->pos()));
	
	if(e->button()==Qt::RightButton && clickIdx.isValid()) {
		QModelIndex nameIdx(clickIdx.sibling(clickIdx.row(), 0));
		bool shown=model()->data(clickIdx, FunctionsModel::Shown).toBool();
		QString actuallyShown;
		QString icon;
		if(shown) {
			icon="user-invisible";
			actuallyShown=i18n("Hide '%1'", model()->data(nameIdx).toString());
		} else {
			icon="user-online";
			actuallyShown=i18n("Show '%1'", model()->data(nameIdx).toString());
		}
		
		QMenu menu(this);
		QAction* actionShown=menu.addAction(KIcon(icon), actuallyShown);
		QAction* actionRemove=menu.addAction(KIcon("list-remove"), i18n("Remove '%1'", model()->data(nameIdx).toString()));
		QAction* result=menu.exec(e->globalPos());
		if(result==actionShown) {
			model()->setData(clickIdx, !shown, FunctionsModel::Shown);
		} else if(result==actionRemove) {
			model()->removeRow(clickIdx.row());
		}
	} else {
		QTreeView::mousePressEvent(e);
	}
}

#include "functionsview.moc"
