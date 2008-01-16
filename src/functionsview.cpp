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
#include <QMouseEvent>
#include <QMenu>

FunctionsView::FunctionsView(QWidget * parent) : QTreeView(parent)
{
}

void FunctionsView::selectionChanged(const QItemSelection & selected, const QItemSelection &)
{
	QModelIndex idx=selected.indexes().first();
	model()->setData(idx, QVariant(), FunctionsModel::Selection);
}

void FunctionsView::mousePressEvent(QMouseEvent * e)
{
	QModelIndex idx(indexAt(e->pos()));
	qDebug() << "press" << e;
	if(e->button()==Qt::RightButton && idx.isValid()) {
		bool shown=model()->data(idx, FunctionsModel::Shown).toBool();
		QString actuallyShown;
		if(shown)
			actuallyShown=i18n("Hide '%1'", model()->data(idx).toString());
		else
			actuallyShown=i18n("Show '%1'", model()->data(idx).toString());
		
		QMenu menu(this);
		QAction* actionShown=menu.addAction(actuallyShown);
		QAction* result=menu.exec(e->globalPos());
		if(actionShown==result) {
			model()->setData(idx, !shown, FunctionsModel::Shown);
		}
	}
}

#include "functionsview.moc"
