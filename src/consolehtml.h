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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QWebEngineView>
#include <QWidget>

#include "consolemodel.h"
#include <analitza/analyzer.h>

class ConsoleModel;

class InlineOptions
{
public:
    virtual ~InlineOptions()
    {
    }

    virtual QString id() const = 0;
    virtual QString caption() const = 0;
    virtual bool matchesExpression(const Analitza::Expression &exp) const = 0;
    virtual void triggerOption(const Analitza::Expression &exp) = 0;
};

/**
 *    The Console widget is able to receive an operation, solve it and show the value.
 *    It also is able to load scripts and save logs.
 *    @author Aleix Pol Gonzalez
 */

class ConsoleHtml : public QWebEngineView
{
    Q_OBJECT
public:
    /** Constructor. Creates a console widget. */
    ConsoleHtml(QWidget *parent = nullptr);

    /** Destructor. */
    ~ConsoleHtml() override;

    /** Retrieves a pointer to the Analitza calculator associated. */
    Analitza::Analyzer *analitza();

    /** Sets a @p newMode console mode. */
    void setMode(ConsoleModel::ConsoleMode newMode);

    /** Retrieves the console mode. */
    ConsoleModel::ConsoleMode mode() const;

    void addOptionsObserver(InlineOptions *opt)
    {
        m_options += opt;
    }

    void contextMenuEvent(QContextMenuEvent *ev) override;

public Q_SLOTS:
    /** Adds the operation defined by the expression @p e. */
    bool addOperation(const Analitza::Expression &e, const QString &input);

    /** Loads a script from @p path. */
    bool loadScript(const QUrl &path);

    /** Save a script yo @p path. */
    bool saveScript(const QUrl &path) const;

    /** Saves a log to @p path. */
    bool saveLog(const QUrl &path) const;

    /** Flushes the contents. */
    void clear();

    /** Copies the selected text to the clipboard */
    void copy() const;

    void openClickedUrl(const QUrl &url);

    void setActualUrl(const QUrl &url);

Q_SIGNALS:
    /** Emits a notification that tells that the widget status. */
    void status(const QString &msg);

    /** Emits that something has changed. */
    void changed();

    /** Emits the selected code to be pasted somewhere */
    void paste(const QString &code);

private Q_SLOTS:
    void modifyVariable(const QString &name, const Analitza::Expression &exp);
    void removeVariable(const QString &name);
    void paste();

private:
    void includeOperation(const Analitza::Expression &expression, const Analitza::Expression &result);
    void updateView();

    QString m_optionsString;
    QUrl m_actualUrl;
    QList<InlineOptions *> m_options;
    QScopedPointer<ConsoleModel> m_model;
};

#endif
