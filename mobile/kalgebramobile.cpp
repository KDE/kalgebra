// SPDX-FileCopyrightText: 2010 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "kalgebramobile.h"

#include <analitza/expression.h>
#include <analitzaplot/plotsmodel.h>

#include "../src/consolemodel.h"

#include "clipboard.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <qqml.h>

using namespace Analitza;

KAlgebraMobile *KAlgebraMobile::s_self = nullptr;
KAlgebraMobile *KAlgebraMobile::self()
{
    return s_self;
}

Q_DECLARE_METATYPE(QSharedPointer<Analitza::Variables>)

KAlgebraMobile::KAlgebraMobile(QObject *parent)
    : QObject(parent)
    , m_functionsModel(nullptr)
    , m_vars(new Analitza::Variables)
{
    Q_ASSERT(s_self == nullptr);
    s_self = this;

    const auto uri = "org.kde.kalgebra.mobile";
    qmlRegisterType<ConsoleModel>("org.kde.kalgebra.mobile", 1, 0, "ConsoleModel");
    qmlRegisterType<QSortFilterProxyModel>("org.kde.kalgebra.mobile", 1, 0, "QSortFilterProxyModel");
    qmlRegisterUncreatableType<QAbstractItemModel>("org.kde.kalgebra.mobile", 1, 0, "QAbstractItemModel", QStringLiteral("no"));
    qmlRegisterType<Clipboard>(uri, 1, 0, "Clipboard");

    qmlRegisterAnonymousType<QStandardItemModel>("Kalgebra", 1);
    qmlRegisterUncreatableType<Analitza::Expression>("org.kde.kalgebra.mobile", 1, 0, "Expression", QStringLiteral("because"));
    qRegisterMetaType<QSharedPointer<Analitza::Variables>>("QSharedPointer<Analitza::Variables>");
}

PlotsModel *KAlgebraMobile::functionsModel()
{
    if (!m_functionsModel) {
        m_functionsModel = new Analitza::PlotsModel(this);
        connect(m_functionsModel, &QAbstractItemModel::rowsRemoved, this, &KAlgebraMobile::functionRemoved);
        connect(m_functionsModel, &QAbstractItemModel::rowsInserted, this, &KAlgebraMobile::functionInserted);
        connect(m_functionsModel, &QAbstractItemModel::dataChanged, this, &KAlgebraMobile::functionModified);
    }

    return m_functionsModel;
}

void KAlgebraMobile::functionInserted(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());
    for (int i = start; i <= end; i++) {
        QModelIndex idx = functionsModel()->index(i, 1);
        m_vars->modify(idx.sibling(i, 0).data().toString(), Analitza::Expression(idx.data().toString()));
    }
}

void KAlgebraMobile::functionRemoved(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());
    for (int i = start; i <= end; i++) {
        QModelIndex idx = functionsModel()->index(i);
        m_vars->remove(functionsModel()->data(idx, Qt::DisplayRole).toString());
    }
}

void KAlgebraMobile::functionModified(const QModelIndex &idxA, const QModelIndex &idxB)
{
    if (idxB.column() == 1) {
        for (int i = idxA.row(); i < idxB.row(); ++i) {
            QModelIndex idx = functionsModel()->index(i, 1);
            m_vars->modify(idx.sibling(i, 0).data().toString(), Analitza::Expression(idx.data().toString()));
        }
    } // else TODO: figure out how to control a "rename"
}

QSharedPointer<Analitza::Variables> KAlgebraMobile::variables() const
{
    return m_vars;
}

#include "moc_kalgebramobile.cpp"
