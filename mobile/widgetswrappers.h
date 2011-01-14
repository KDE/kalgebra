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

#ifndef WIDGETSWRAPPERS_H
#define WIDGETSWRAPPERS_H

#include <QListWidget>
#include <QTreeView>

class VerticalLayout : public QWidget
{
	Q_OBJECT
	public:
		VerticalLayout(const QWidget* w=0);
		
	public slots:
		void addWidget(QWidget* w);
};

class ListWidget : public QListWidget
{
	Q_OBJECT
	public:
		explicit ListWidget(QWidget* parent = 0);
		
	public slots:
		void addItem(const QString& item);
		void clear();
};

class TreeView : public QTreeView
{
	Q_OBJECT
	public:
		explicit TreeView(QWidget* parent = 0);
		Q_SCRIPTABLE virtual void setModel(QObject* model);
};

#endif
