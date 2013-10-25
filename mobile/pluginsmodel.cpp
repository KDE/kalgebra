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


#include "pluginsmodel.h"
#include <KConfigGroup>
#include <KConfig>
#include <QIcon>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>

PluginsModel::PluginsModel(QObject* parent) :QStandardItemModel(parent)
{
	QStringList foundPlugins = QStandardPaths::locateAll(QStandardPaths::DataLocation, "plugins/*.desktop");

	qDebug() << "Plugins found:" << foundPlugins;

	QList<QStandardItem*> items;
	Q_FOREACH(const QString& file, foundPlugins) {
		KConfig info(file, KConfig::SimpleConfig, QStandardPaths::DataLocation);
		KConfigGroup cg = info.group("Desktop Entry");
		QStandardItem* item = new QStandardItem;

		QString postfix = "plugins/"+cg.readEntry("X-KDE-PluginInfo-Name", QString());
		QString scriptPath = QStandardPaths::locate(QStandardPaths::DataLocation, postfix);

		Q_ASSERT(!scriptPath.isEmpty());

		QVariant priority = cg.readEntry("X-KAlgebra-Priority", QString());
		if(!priority.isValid())
			priority = 1000;
		
		item->setData(scriptPath, PathRole);
		item->setData(priority, PriorityRole);
		item->setData(cg.readEntry("Name", QString()), TitleRole);
		item->setData(cg.readEntry("Comment", QString()), SubtitleRole);
		item->setData(cg.readEntry("Icon", QString()), Qt::DecorationRole);
		
		items += item;
	}
	invisibleRootItem()->appendRows(items);
	setSortRole(PriorityRole);
	sort(0, Qt::DescendingOrder);
}

QHash<int, QByteArray> PluginsModel::roleNames() const
{
	QHash<int, QByteArray> rolenames=QStandardItemModel::roleNames();
	rolenames.insert(PathRole, "path");
	rolenames.insert(PriorityRole, "priority");
	rolenames.insert(TitleRole, "title");
	rolenames.insert(SubtitleRole, "subtitle");
	return rolenames;
}

QString PluginsModel::pluginPath(int row)
{
	return data(index(row, 0), PathRole).toString();
}
