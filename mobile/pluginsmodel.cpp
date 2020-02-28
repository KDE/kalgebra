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
#include <QDir>
#include <QDebug>
#include <QUrl>
#include <QStandardPaths>
#include <KPluginMetaData>

QString PluginsModel::pluginsDirectoryPath()
{
#ifdef __ANDROID__
    return QStringLiteral("assets:/share/kalgebramobile/plugins");
#else
    return QStandardPaths::locate(QStandardPaths::DataLocation, QStringLiteral("plugins"), QStandardPaths::LocateDirectory);
#endif
}

PluginsModel::PluginsModel(QObject* parent) :QStandardItemModel(parent)
{
    QStringList foundPlugins;
    QDir dir(pluginsDirectoryPath());
    foreach(const auto& file, dir.entryInfoList(QStringList("*.json"))) {
        foundPlugins += file.absoluteFilePath();
    }

    qDebug() << "Plugins found:" << foundPlugins;

    QList<QStandardItem*> items;
    Q_FOREACH(const QString& file, foundPlugins) {
        KPluginMetaData md(file);
        if(!md.isValid()) {
            qWarning() << "error opening " << file;
            continue;
        }
//         qDebug() << "laaaaa" << f.readAll() << file;

        QStandardItem* item = new QStandardItem;

        QString scriptPath = dir.absoluteFilePath(md.value(QStringLiteral("X-KDE-PluginInfo-Name"), QString()));

        Q_ASSERT(!scriptPath.isEmpty());

        const auto priority = md.value(QStringLiteral("X-KAlgebra-Priority"), QStringLiteral("1000")).toInt();
        
        item->setData(QUrl::fromLocalFile(scriptPath), PathRole);
        item->setData(priority, PriorityRole);
        item->setData(md.name(), TitleRole);
        item->setData(md.description(), SubtitleRole);
        item->setData(md.iconName(), Qt::DecorationRole);
        
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

QStringList PluginsModel::titles() const
{
    QStringList ret;
    for (int i=0, c=rowCount(); i<c; ++i) {
        ret += item(i, 0)->data(TitleRole).toString();
    }
    return ret;
}
