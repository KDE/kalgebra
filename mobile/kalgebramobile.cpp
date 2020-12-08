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

#include "kalgebramobile.h"

#include <analitzaplot/plotsmodel.h>
#include <analitza/variables.h>
#include <analitza/expression.h>

#include "../src/consolemodel.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <qqml.h>
#include "clipboard.h"

using namespace Analitza;

KAlgebraMobile* KAlgebraMobile::s_self=0;
KAlgebraMobile* KAlgebraMobile::self() { return s_self; }

Q_DECLARE_METATYPE(QSharedPointer<Analitza::Variables>)

KAlgebraMobile::KAlgebraMobile(QObject* parent)
    : QObject(parent), m_functionsModel(0), m_vars(new Analitza::Variables)
{
    Q_ASSERT(s_self==0);
    s_self=this;
    
    const auto uri = "org.kde.kalgebra.mobile";
    qmlRegisterType<ConsoleModel>("org.kde.kalgebra.mobile", 1, 0, "ConsoleModel");
    qmlRegisterType<QSortFilterProxyModel>("org.kde.kalgebra.mobile", 1, 0, "QSortFilterProxyModel");
    qmlRegisterUncreatableType<QAbstractItemModel>("org.kde.kalgebra.mobile", 1, 0, "QAbstractItemModel", "no");
    qmlRegisterType<Clipboard>(uri, 1, 0, "Clipboard");

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<QStandardItemModel>();
#else
    qmlRegisterAnonymousType<QStandardItemModel>("Kalgebra", 1);
#endif
    qmlRegisterUncreatableType<Analitza::Expression>("org.kde.kalgebra.mobile", 1, 0, "Expression", QStringLiteral("because"));
    qRegisterMetaType<QSharedPointer<Analitza::Variables>>("QSharedPointer<Analitza::Variables>");
}

PlotsModel* KAlgebraMobile::functionsModel()
{
    if(!m_functionsModel) {
        m_functionsModel = new Analitza::PlotsModel(this);
        connect(m_functionsModel, &QAbstractItemModel::rowsRemoved, this, &KAlgebraMobile::functionRemoved);
        connect(m_functionsModel, &QAbstractItemModel::rowsInserted, this, &KAlgebraMobile::functionInserted);
        connect(m_functionsModel, &QAbstractItemModel::dataChanged, this, &KAlgebraMobile::functionModified);
    }
    
    return m_functionsModel;
}

void KAlgebraMobile::functionInserted(const QModelIndex& parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());
    for(int i=start; i<=end; i++) {
        QModelIndex idx = functionsModel()->index(i, 1);
        m_vars->modify(idx.sibling(i,0).data().toString(), Analitza::Expression(idx.data().toString()));
    }
}

void KAlgebraMobile::functionRemoved(const QModelIndex& parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());
    for(int i=start; i<=end; i++) {
        QModelIndex idx = functionsModel()->index(i);
        m_vars->remove(functionsModel()->data(idx, Qt::DisplayRole).toString());
    }
}

void KAlgebraMobile::functionModified(const QModelIndex& idxA, const QModelIndex& idxB)
{
    if(idxB.column()==1) {
        for(int i=idxA.row(); i<idxB.row(); ++i) {
            QModelIndex idx = functionsModel()->index(i, 1);
            m_vars->modify(idx.sibling(i,0).data().toString(), Analitza::Expression(idx.data().toString()));
        }
    } //else TODO: figure out how to control a "rename"
}

QSharedPointer<Analitza::Variables> KAlgebraMobile::variables() const { return m_vars; }
