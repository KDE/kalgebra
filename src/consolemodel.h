/*************************************************************************************
 *  Copyright (C) 2007-2017 by Aleix Pol <aleixpol@kde.org>                          *
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

#ifndef CONSOLEMODEL_H
#define CONSOLEMODEL_H

#include <QObject>
#include <analitza/analyzer.h>
#include <analitza/expression.h>
#include <analitza/variables.h>

class ConsoleModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ConsoleMode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QSharedPointer<Analitza::Variables> variables READ variables WRITE setVariables NOTIFY variablesChanged)
public:
    ConsoleModel(QObject *parent = nullptr);

    /** This enumeration controles the way the console will calculate and show his results. */
    enum ConsoleMode {
        Evaluation, /**< Simplifies the expression, tries to simplify when sees a variable not defined. */
        Calculation /**< Calculates everything, if it finds a not defined variable shows an error. */
    };
    Q_ENUM(ConsoleMode)

    Q_SCRIPTABLE bool addOperation(const QString &input);
    bool addOperation(const Analitza::Expression &e, const QString &input);

    Q_SCRIPTABLE bool loadScript(const QUrl &path);
    Q_SCRIPTABLE bool saveScript(const QUrl &path);
    Q_SCRIPTABLE void clear();
    Q_SCRIPTABLE bool saveLog(const QUrl &path) const;

    Q_SCRIPTABLE static QString readContent(const QUrl &url);

    QByteArray css() const;

    ConsoleMode mode() const
    {
        return m_mode;
    }
    void setMode(ConsoleMode mode);

    QSharedPointer<Analitza::Variables> variables() const
    {
        return a.variables();
    }
    void setVariables(const QSharedPointer<Analitza::Variables> &vars);
    Analitza::Analyzer *analyzer()
    {
        return &a;
    }

    QList<QByteArray> htmlLog() const
    {
        return m_htmlLog;
    }

Q_SIGNALS:
    void message(const QString &msg, const Analitza::Expression &operation, const Analitza::Expression &result);
    void updateView();
    void modeChanged(ConsoleModel::ConsoleMode mode);
    void operationSuccessful(const Analitza::Expression &expression, const Analitza::Expression &result);
    void variablesChanged();

private:
    void addMessage(const QString &msg, const Analitza::Expression &operation, const Analitza::Expression &result);

    QList<QByteArray> m_htmlLog;
    Analitza::Analyzer a;
    ConsoleMode m_mode = Evaluation;
    QList<Analitza::Expression> m_script;
};

#endif
