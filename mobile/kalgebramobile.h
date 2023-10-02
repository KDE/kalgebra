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

#ifndef KALGEBRAMOBILE_H
#define KALGEBRAMOBILE_H

#include <QObject>
#include <QSharedPointer>
#include <analitza/variables.h>

class QModelIndex;
namespace Analitza
{
class PlotsModel;
}

class KAlgebraMobile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSharedPointer<Analitza::Variables> variables READ variables NOTIFY variablesChanged)
public:
    explicit KAlgebraMobile(QObject *parent = nullptr);

    static KAlgebraMobile *self();
    void notifyVariablesChanged()
    {
        variablesChanged();
    }

public Q_SLOTS:
    Analitza::PlotsModel *functionsModel();
    QSharedPointer<Analitza::Variables> variables() const;

private Q_SLOTS:
    void functionRemoved(const QModelIndex &parent, int start, int end);
    void functionModified(const QModelIndex &idxA, const QModelIndex &idxB);
    void functionInserted(const QModelIndex &parent, int start, int end);

Q_SIGNALS:
    void variablesChanged();

private:
    static KAlgebraMobile *s_self;

    Analitza::PlotsModel *m_functionsModel;
    QSharedPointer<Analitza::Variables> m_vars;
};

#endif // KALGEBRAMOBILE_H
