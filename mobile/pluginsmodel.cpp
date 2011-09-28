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

	KStandardDirs d;
	QStringList basedirs = d.findDirs("data", "kalgebra/plugins");
	QStringList foundPlugins;
	foreach(const QString& dir, basedirs) {
		//we list <dir>/*/*.desktop

		QDir d(dir);
		QStringList files = d.entryList(QStringList("*.desktop"));

		foreach(const QString& plugin, files) {
			foundPlugins += d.absoluteFilePath(plugin);
		}
	}

	qDebug() << "Plugins found:" << foundPlugins;
	m_plugins = KPluginInfo::fromFiles(foundPlugins);
	setSortRole(PriorityRole);

	Q_FOREACH(const KPluginInfo& info, m_plugins) {
// 				const KPluginInfo& info;
		QStandardItem* item = new QStandardItem(QIcon::fromTheme(info.icon()), info.name());

		QString postfix = "kalgebra/plugins/"+info.pluginName();
		QString scriptPath = KStandardDirs::locate("data", postfix);

		Q_ASSERT(!scriptPath.isEmpty());

		QVariant priority = info.property("X-KAlgebra-Priority");
		if(!priority.isValid())
			priority = 1000;
		item->setData(scriptPath, PathRole);
		item->setData(priority, PriorityRole);
		item->setData(info.name(), TitleRole);
		item->setData(info.comment(), SubtitleRole);

		appendRow(item);
	}
	setSortRole(PriorityRole);
    sort(0);
}

QString PluginsModel::pluginPath(int row)
{
	return data(index(row, 0), PathRole).toString();
}
