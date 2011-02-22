/*************************************************************************************
 *  Copyright (C) 2010 by Aleix Pol <aleixpol@kde.org>                               *
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

#include "widgetswrappers.h"
#include <QVBoxLayout>
#include <QScrollBar>

VerticalLayout::VerticalLayout(const QWidget* w) { setLayout(new QVBoxLayout); }
void VerticalLayout::addWidget(QWidget* w) { layout()->addWidget(w); }

////////////////////////////////////

ListWidget::ListWidget(QWidget* parent)
	: QListWidget(parent)
{setInputMethodHints(Qt::ImhNoAutoUppercase);}

void ListWidget::addItem(const QString& item) { QListWidget::addItem(item); }
void ListWidget::clear() { QListWidget::clear(); }
void ListWidget::scrollDown() { verticalScrollBar()->setValue(verticalScrollBar()->maximum()); }

////////////////////////////////////

TreeView::TreeView(QWidget* parent): QTreeView(parent)
{}

void TreeView::setModel(QObject* model)
{
    QTreeView::setModel(qobject_cast<QAbstractItemModel*>(model));
}
