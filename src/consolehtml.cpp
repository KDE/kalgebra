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

#include "consolehtml.h"

#include <QClipboard>
#include <QApplication>
#include <QEvent>
#include <QUrlQuery>
#include <QTemporaryFile>
#include <QDir>
#include <QContextMenuEvent>

#include <KStandardAction>
#include <QMenu>
#include <KLocalizedString>
#include <QAction>
#include <kio/copyjob.h>
#include <kio/job.h>

#include <analitza/variables.h>
#include <analitza/expression.h>

static QUrl temporaryPath()
{
    QTemporaryFile temp(QStringLiteral("consolelog"));
    temp.open();
    temp.close();
    temp.setAutoRemove(false);
    return QUrl::fromLocalFile(temp.fileName());
}

static QUrl retrieve(const QUrl& remoteUrl)
{
    const QUrl path = temporaryPath();
    KIO::CopyJob* job=KIO::copyAs(remoteUrl, path);

    job->exec();

    return path;
}

class ConsolePage : public QWebEnginePage
{
public:
    ConsolePage(ConsoleHtml* parent) : QWebEnginePage(parent), m_console(parent) {}

    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool /*isMainFrame*/) override {
        if (url.scheme() == QLatin1String("data"))
            return true;
        qDebug() << "navigating to" << url << type;
        m_console->openClickedUrl(url);
        return false;
    }

    ConsoleHtml* m_console;
};

ConsoleHtml::ConsoleHtml(QWidget *parent)
    : QWebEngineView(parent)
    , m_model(new ConsoleModel(this))
{
    connect(m_model.data(), &ConsoleModel::updateView, this, &ConsoleHtml::updateView);
    connect(m_model.data(), &ConsoleModel::operationSuccessful, this, &ConsoleHtml::includeOperation);
    setPage(new ConsolePage(this));
}

ConsoleHtml::~ConsoleHtml()
{
    qDeleteAll(m_options);
}

void ConsoleHtml::openClickedUrl(const QUrl& url)
{
    QUrlQuery query(url);
    QString id =query.queryItemValue(QStringLiteral("id"));
    QString exp=query.queryItemValue(QStringLiteral("func"));
    
    if(id==QLatin1String("copy")) {
        emit paste(exp);
    } else foreach(InlineOptions* opt, m_options) {
        if(opt->id() == id) {
            opt->triggerOption(Analitza::Expression(exp, false));
        }
    }
}

bool ConsoleHtml::addOperation(const Analitza::Expression& e, const QString& input)
{
    return m_model->addOperation(e, input);
}

ConsoleModel::ConsoleMode ConsoleHtml::mode() const
{
    return m_model->mode();
}

void ConsoleHtml::setMode(ConsoleModel::ConsoleMode newMode)
{
    m_model->setMode(newMode);
}

Analitza::Analyzer* ConsoleHtml::analitza()
{
    return m_model->analyzer();
}

bool ConsoleHtml::loadScript(const QUrl& path)
{
    return m_model->loadScript(path.isLocalFile() ? path : retrieve(path));
}

bool ConsoleHtml::saveScript(const QUrl & path) const
{
    const QUrl savePath=path.isLocalFile() ? path : temporaryPath();
    bool correct = m_model->saveScript(savePath);
    if(!path.isLocalFile()) {
        KIO::CopyJob* job=KIO::move(savePath, path);
        correct=job->exec();
    }
    return correct;
}

bool ConsoleHtml::saveLog(const QUrl & path) const
{
    const QUrl savePath=path.isLocalFile() ? path : temporaryPath();
    bool correct = m_model->saveLog(savePath);
    if(!path.isLocalFile()) {
        KIO::CopyJob* job=KIO::move(savePath, path);
        correct=job->exec();
    }
    return correct;
}

void ConsoleHtml::includeOperation(const Analitza::Expression& /*e*/, const Analitza::Expression& res)
{
    m_optionsString.clear();
    if (res.isCorrect()) {
        Analitza::Analyzer lambdifier(m_model->variables());
        lambdifier.setExpression(res);
        Analitza::Expression lambdaexp = lambdifier.dependenciesToLambda();
        lambdifier.setExpression(lambdaexp);

        foreach(InlineOptions* opt, m_options) {
            if(opt->matchesExpression(lambdaexp)) {
                QUrl url(QStringLiteral("/query"));
                QUrlQuery query(url);
                query.addQueryItem(QStringLiteral("id"), opt->id());
                query.addQueryItem(QStringLiteral("func"), lambdaexp.toString());
                url.setQuery(query);

                m_optionsString += i18n(" <a href='kalgebra:%1'>%2</a>", url.toString(), opt->caption());
            }
        }

        if(!m_optionsString.isEmpty()) {
            m_optionsString = "<div class='options'>"+i18n("Options: %1", m_optionsString)+"</div>";
        }
    }
}

void ConsoleHtml::updateView()
{
    QByteArray code;
    code += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    code += "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n<head>\n\t<title> :) </title>\n";
    code += m_model->css();
    code += "</head>\n<body>\n";

    auto log = m_model->htmlLog();
    if (!log.isEmpty()) {
        const auto newEntry = log.takeLast();
        foreach(const QString &entry, log)
            code += "<p class='normal'>" + entry.toUtf8() + "</p>\n";

        code += m_optionsString.toUtf8();
        if (newEntry.startsWith("<ul class='error'>"))
            code += newEntry;
        else
            code += "<p class='last'>" + newEntry + "</p>\n";
    }
    code += "</body></html>";

    page()->setHtml(code);

    emit changed();

    connect(this, &QWebEngineView::loadFinished, this, [this](bool ok){
        if (!ok) {
            qWarning() << "error loading page" << url();
        }
        page()->runJavaScript(QStringLiteral("window.scrollTo(0, document.body.scrollHeight);"));
    });
}

void ConsoleHtml::copy() const
{
    QApplication::clipboard()->setText(selectedText());
}

void ConsoleHtml::contextMenuEvent(QContextMenuEvent* ev)
{
    QMenu popup;
    if(hasSelection()) {
        popup.addAction(KStandardAction::copy(this, SLOT(copy()), &popup));
        QAction *act=new QAction(QIcon::fromTheme(QStringLiteral("edit-paste")), i18n("Paste \"%1\" to input", selectedText().trimmed()), &popup);
        connect(act, SIGNAL(triggered()), SLOT(paste()));
        popup.addAction(act);
        popup.addSeparator();
    }
    popup.addAction(KStandardAction::clear(this, SLOT(clear()), &popup));
    
    popup.exec(mapToGlobal(ev->pos()));
}

void ConsoleHtml::clear()
{
    m_model->clear();
    updateView();
}

void ConsoleHtml::modifyVariable(const QString& name, const Analitza::Expression& exp)
{
    m_model->variables()->modify(name, exp);
}

void ConsoleHtml::removeVariable(const QString & name)
{
    m_model->variables()->remove(name);
}

void ConsoleHtml::paste()
{
    emit paste(selectedText().trimmed());
}
