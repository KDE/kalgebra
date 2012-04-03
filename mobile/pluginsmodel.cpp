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
#include <KStandardDirs>
#include <KDesktopFile>
#include <KConfigGroup>
#include <QIcon>
#include <QDir>
#include <QDebug>

PluginsModel::PluginsModel(QObject* parent) :QStandardItemModel(parent)
{
	QHash<int, QByteArray> rolenames=QStandardItemModel::roleNames();
	rolenames.insert(PathRole, "path");
	rolenames.insert(PriorityRole, "priority");
	rolenames.insert(TitleRole, "title");
	rolenames.insert(SubtitleRole, "subtitle");
	setRoleNames(rolenames);

	qDebug() << "fuuu" << KGlobal::dirs()->findDirs("appdata", "plugins");
	QStringList foundPlugins = KGlobal::dirs()->findAllResources("appdata", "plugins/*.desktop");

	qDebug() << "Plugins found:" << foundPlugins;

	QList<QStandardItem*> items;
	Q_FOREACH(const QString& file, foundPlugins) {
		KDesktopFile info(file);
		KConfigGroup cg = info.desktopGroup();
		QStandardItem* item = new QStandardItem;

		QString postfix = "plugins/"+cg.readEntry("X-KDE-PluginInfo-Name", QString());
		QString scriptPath = KStandardDirs::locate("appdata", postfix);

		Q_ASSERT(!scriptPath.isEmpty());

		QVariant priority = cg.readEntry("X-KAlgebra-Priority", QString());
		if(!priority.isValid())
			priority = 1000;
		
		item->setData(scriptPath, PathRole);
		item->setData(priority, PriorityRole);
		item->setData(info.readName(), TitleRole);
		item->setData(cg.readEntry("Comment", QString()), SubtitleRole);
		item->setData(cg.readEntry("Icon", QString()), Qt::DecorationRole);
		
		items += item;
	}
	invisibleRootItem()->appendRows(items);
	setSortRole(PriorityRole);
	sort(0, Qt::DescendingOrder);
}

QString PluginsModel::pluginPath(int row)
{
	return data(index(row, 0), PathRole).toString();
}
