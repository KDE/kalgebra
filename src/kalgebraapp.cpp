// SPDX-FileCopyrightText: 2010 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "kalgebraapp.h"

#include <analitza/expression.h>
#include <analitzaplot/plotsmodel.h>

#include "consolemodel.h"

#include "clipboard.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <qqml.h>

using namespace Analitza;

KAlgebraApp *KAlgebraApp::s_self = nullptr;
KAlgebraApp *KAlgebraApp::self()
{
    return s_self;
}

Q_DECLARE_METATYPE(QSharedPointer<Analitza::Variables>)

KAlgebraApp::KAlgebraApp(QObject *parent)
    : QObject(parent)
    , m_functionsModel(nullptr)
    , m_vars(new Analitza::Variables)
{
    Q_ASSERT(s_self == nullptr);
    s_self = this;

    const auto uri = "org.kde.kalgebra";
    qmlRegisterType<ConsoleModel>(uri, 1, 0, "ConsoleModel");
    qmlRegisterType<QSortFilterProxyModel>(uri, 1, 0, "QSortFilterProxyModel");
    qmlRegisterUncreatableType<QAbstractItemModel>(uri, 1, 0, "QAbstractItemModel", QStringLiteral("no"));
    qmlRegisterType<Clipboard>(uri, 1, 0, "Clipboard");

    qmlRegisterAnonymousType<QStandardItemModel>("Kalgebra", 1);
    qmlRegisterUncreatableType<Analitza::Expression>(uri, 1, 0, "Expression", QStringLiteral("because"));
    qRegisterMetaType<QSharedPointer<Analitza::Variables>>("QSharedPointer<Analitza::Variables>");
}

PlotsModel *KAlgebraApp::functionsModel()
{
    if (!m_functionsModel) {
        m_functionsModel = new Analitza::PlotsModel(this);
        connect(m_functionsModel, &QAbstractItemModel::rowsRemoved, this, &KAlgebraApp::functionRemoved);
        connect(m_functionsModel, &QAbstractItemModel::rowsInserted, this, &KAlgebraApp::functionInserted);
        connect(m_functionsModel, &QAbstractItemModel::dataChanged, this, &KAlgebraApp::functionModified);
    }

    return m_functionsModel;
}

void KAlgebraApp::functionInserted(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());
    for (int i = start; i <= end; i++) {
        QModelIndex idx = functionsModel()->index(i, 1);
        m_vars->modify(idx.sibling(i, 0).data().toString(), Analitza::Expression(idx.data().toString()));
    }
}

void KAlgebraApp::functionRemoved(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(!parent.isValid());
    for (int i = start; i <= end; i++) {
        QModelIndex idx = functionsModel()->index(i);
        m_vars->remove(functionsModel()->data(idx, Qt::DisplayRole).toString());
    }
}

void KAlgebraApp::functionModified(const QModelIndex &idxA, const QModelIndex &idxB)
{
    if (idxB.column() == 1) {
        for (int i = idxA.row(); i < idxB.row(); ++i) {
            QModelIndex idx = functionsModel()->index(i, 1);
            m_vars->modify(idx.sibling(i, 0).data().toString(), Analitza::Expression(idx.data().toString()));
        }
    } // else TODO: figure out how to control a "rename"
}

QSharedPointer<Analitza::Variables> KAlgebraApp::variables() const
{
    return m_vars;
}

#include "moc_kalgebraapp.cpp"
