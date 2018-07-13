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


#ifndef PLUGINSMODEL_H
#define PLUGINSMODEL_H

#include <QStandardItemModel>

class PluginsModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList titles READ titles CONSTANT)
    public:
        enum Roles { PathRole = Qt::UserRole+1, PriorityRole, TitleRole, SubtitleRole };
        Q_ENUM(Roles)

        explicit PluginsModel(QObject* parent = 0);
        QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;
        
        static QString pluginsDirectoryPath();

        QStringList titles() const;

    public Q_SLOTS:
        ///qml can't access data. Yay!
        QString pluginPath(int row);
};

#endif // PLUGINSMODEL_H
