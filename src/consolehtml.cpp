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
#include <qevent.h>
#include <QUrlQuery>
#include <QTemporaryFile>
#include <QDir>

#include <KStandardAction>
#include <QMenu>
#include <KLocalizedString>
#include <QAction>
#include <kio/copyjob.h>
#include <kio/job.h>

#include <analitza/variables.h>
#include <analitza/expression.h>

Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, s_css, (
    "<style type=\"text/css\">\n"
        "\thtml { background-color: " + qApp->palette().color(QPalette::Active, QPalette::Base).name().toLatin1() + "; }\n"
        "\t.error { border-style: solid; border-width: 1px; border-color: #ff3b21; background-color: #ffe9c4; padding:7px;}\n"
        "\t.last  { border-style: solid; border-width: 1px; border-color: #2020ff; background-color: #e0e0ff; padding:7px;}\n"
        "\t.before { text-align:right; }\n"
        "\t.op  { font-weight: bold; }\n"
    //     "\t.normal:hover  { border-style: solid; border-width: 1px; border-color: #777; }\n";
        "\t.normal:hover  { background-color: #f7f7f7; }\n"
        "\t.cont { color: #560000; }\n"
        "\t.num { color: #0000C4; }\n"
        "\t.sep { font-weight: bold; color: #0000FF; }\n"
        "\t.var { color: #640000; }\n"
        "\t.keyword { color: #000064; }\n"
        "\t.func { color: #008600; }\n"
        "\t.result { padding-left: 10%; }\n"
        "\t.options { font-size: small; text-align:right }\n"
        "\t.string { color: #bb0000 }\n"
        "\tli { padding-left: 12px; padding-bottom: 4px; list-style-position: inside; }"
        "\t.exp { color: #000000 }\n"
        "\ta { color: #0000ff }\n"
        "\ta:link {text-decoration:none;}\n"
        "\ta:visited {text-decoration:none;}\n"
        "\ta:hover {text-decoration:underline;}\n"
        "\ta:active {text-decoration:underline;}\n"
        "\tp { font-size: " +QByteArray::number(QFontMetrics(QApplication::font()).height())+ "px; }\n"
        "</style>\n"))

static QString temporaryPath()
{
    QTemporaryFile temp(QStringLiteral("consolelog"));
    temp.open();
    temp.close();
    temp.setAutoRemove(false);
    return temp.fileName();
}

static QString retrieve(const QUrl& remoteUrl)
{
    QString path = temporaryPath();

    KIO::CopyJob* job=KIO::copyAs(remoteUrl, QUrl::fromLocalFile(path));

    job->exec();

    return path;
}

class ConsolePage : public QWebEnginePage
{
public:
    ConsolePage(ConsoleHtml* parent) : QWebEnginePage(parent), m_console(parent) {}

    bool acceptNavigationRequest(const QUrl &url, NavigationType /*type*/, bool /*isMainFrame*/) override {
        m_console->openClickedUrl(url);
        return false;
    }

    ConsoleHtml* m_console;
};

ConsoleHtml::ConsoleHtml(QWidget *parent)
    : QWebEngineView(parent)
    , m_model(new ConsoleModel)
{
    connect(m_model.data(), &ConsoleModel::updateView, this, &ConsoleHtml::updateView);
    connect(m_model.data(), &ConsoleModel::errorMessage, this, &ConsoleHtml::addMessage);
    setPage(new ConsolePage(this));
}

ConsoleHtml::~ConsoleHtml()
{
    qDeleteAll(m_options);
}

void ConsoleHtml::addMessage(const QString& message)
{
    m_htmlLog += message;
}

void ConsoleHtml::openClickedUrl(const QUrl& url)
{
    QUrlQuery query(url);
    QString id =query.queryItemValue(QStringLiteral("id"));
    QString exp=query.queryItemValue(QStringLiteral("func"));
    
    if(id=="copy") {
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
    return m_model->m_mode;
}

void ConsoleHtml::setMode(ConsoleModel::ConsoleMode newMode)
{
    m_model->m_mode = newMode;
}

Analitza::Analyzer* ConsoleHtml::analitza()
{
    return &(m_model->a);
}

bool ConsoleHtml::loadScript(const QUrl& path)
{
    return m_model->loadScript(path.isLocalFile() ? path.toLocalFile() : retrieve(path));
}

bool ConsoleHtml::saveScript(const QUrl & path) const
{
    const QString savePath=path.isLocalFile() ?  path.toLocalFile() : temporaryPath();
    bool correct = m_model->saveScript(savePath);
    if(!path.isLocalFile()) {
        KIO::CopyJob* job=KIO::move(QUrl(savePath), path);
        correct=job->exec();
    }
    return correct;
}

bool ConsoleHtml::saveLog(const QUrl& path) const
{
    Q_ASSERT(!path.isEmpty());
    //FIXME: We have to choose between txt and html
    QString savePath=path.isLocalFile() ?  path.toLocalFile() : temporaryPath();
    QFile file(savePath);
    bool correct = file.open(QIODevice::WriteOnly | QIODevice::Text);

    if(correct) {
        QTextStream out(&file);
        out << "<html>\n<head>" << *s_css << "</head>" << endl;
        out << "<body>" << endl;
        foreach(const QString &entry, m_htmlLog)
            out << "<p>" << entry << "</p>" << endl;
        out << "</body>\n</html>" << endl;
    }

    if(!path.isLocalFile()) {
        KIO::CopyJob* job=KIO::move(QUrl::fromLocalFile(savePath), path);
        correct=job->exec();
    }
    return correct;
}

void ConsoleHtml::updateView(const QString& newEntry, const Analitza::Expression &res)
{
    QString options;
    if (res.isCorrect()) {
        Analitza::Analyzer lambdifier(m_model->a.variables());
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

                options += i18n(" <a href='kalgebra:%1'>%2</a>", url.toString(), opt->caption());
            }
        }

        if(!options.isEmpty())
            options = "<div class='options'>"+i18n("Options: %1", options)+"</div>";
    }

    QByteArray code;
    code += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    code += "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n<head>\n\t<title> :) </title>\n";
    code += *s_css;
    code += "</head>\n<body>";
    foreach(const QString &entry, m_htmlLog)
        code += "<p class='normal'>"+entry.toUtf8()+"</p>";

    if(!newEntry.isEmpty()) {
        m_htmlLog += newEntry;
        code += options.toUtf8();
        code += "<p class='last'>"+newEntry.toUtf8()+"</p>";
    }
    code += "</body></html>";

    page()->setHtml(code);
    
    emit changed();

    QObject* o = new QObject;
    connect(this, &QWebEngineView::loadFinished, o, [this, o](){
        page()->runJavaScript(QStringLiteral("window.scrollTo(0, document.body.scrollHeight);"));
        delete o;
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
        QAction *act=new QAction(QIcon::fromTheme(QStringLiteral("edit-paste")), i18n("Paste \"%1\" to input", selectedText()), &popup);
        connect(act, SIGNAL(triggered()), SLOT(paste()));
        popup.addAction(act);
        popup.addSeparator();
    }
    popup.addAction(KStandardAction::clear(this, SLOT(clear()), &popup));
    
    popup.exec(ev->pos());
}

void ConsoleHtml::clear()
{
    m_model->m_script.clear();
    m_htmlLog.clear();
    updateView(QString(), {});
}

void ConsoleHtml::modifyVariable(const QString& name, const Analitza::Expression& exp)
{
    m_model->a.variables()->modify(name, exp);
}

void ConsoleHtml::removeVariable(const QString & name)
{
    m_model->a.variables()->remove(name);
}

void ConsoleHtml::paste()
{
    emit paste(selectedText());
}

#include "consolehtml.moc"
