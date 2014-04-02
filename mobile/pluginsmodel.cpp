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
#include <QIcon>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QJsonDocument>

QString PluginsModel::pluginsDirectoryPath()
{
#ifdef __ANDROID__
	return "/data/data/org.kde.kalgebramobile/qt-reserved-files/share/kalgebramobile/plugins";
#else
	return QStandardPaths::locate(QStandardPaths::DataLocation, "plugins", QStandardPaths::LocateDirectory);
#endif
}

PluginsModel::PluginsModel(QObject* parent) :QStandardItemModel(parent)
{
	QStringList foundPlugins;
	QDir dir(pluginsDirectoryPath());
	foreach(const QString& file, dir.entryList(QStringList("*.json"))) {
		foundPlugins += dir.absoluteFilePath(file);
	}

	qDebug() << "Plugins found:" << foundPlugins;

	QList<QStandardItem*> items;
	Q_FOREACH(const QString& file, foundPlugins) {
		QFile f(file);
		bool ret = f.open(QIODevice::ReadOnly);
		if(!ret) {
			qWarning() << "error opening " << file;
			continue;
		}
// 		qDebug() << "laaaaa" << f.readAll() << file;

		QByteArray data = f.readAll();
		QJsonDocument doc = QJsonDocument::fromJson(data);

		QVariantMap cg = doc.toVariant().toMap();
		QStandardItem* item = new QStandardItem;

		QString scriptPath = dir.absoluteFilePath(cg.value("X-KDE-PluginInfo-Name", QString()).toString());

		Q_ASSERT(!scriptPath.isEmpty());

		QVariant priority = cg.value("X-KAlgebra-Priority", QString());
		if(!priority.isValid())
			priority = 1000;
		
		item->setData(scriptPath, PathRole);
		item->setData(priority, PriorityRole);
		item->setData(cg.value("Name", QString()), TitleRole);
		item->setData(cg.value("Comment", QString()), SubtitleRole);
		item->setData(cg.value("Icon", QString()), Qt::DecorationRole);
		
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
