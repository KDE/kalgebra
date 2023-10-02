/*************************************************************************************
 *  Copyright (C) 2007 by Aleix Pol <aleixpol@kde.org>                               *
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

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <QSortFilterProxyModel>
#include <QWidget>

namespace Analitza
{
class Variables;
class PlotsView2D;
class PlotsModel;
}

class QLabel;
class QModelIndex;
class OperatorsModel;

/**
    @author Aleix Pol
*/
class Dictionary : public QWidget
{
    Q_OBJECT
public:
    Dictionary(QWidget *p = nullptr);
    virtual ~Dictionary();

    QSortFilterProxyModel *model() const
    {
        return m_sortProxy;
    }

public Q_SLOTS:
    void activated(const QModelIndex &prev, const QModelIndex &);
    void setFilter(const QString &);

private:
    QLabel *m_name;
    QLabel *m_descr;
    QLabel *m_sample;
    QLabel *m_example;

    Analitza::PlotsView2D *m_graph;
    Analitza::PlotsModel *m_funcs;
    OperatorsModel *m_ops;
    QSharedPointer<Analitza::Variables> m_vars;
    QSortFilterProxyModel *m_sortProxy;
};

#endif
