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

#include "consolemodel.h"

#include <KLocalizedString>
#include <QFile>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QPalette>
#include <QUrl>
#include <QUrlQuery>

using namespace Qt::Literals::StringLiterals;

Q_GLOBAL_STATIC_WITH_ARGS(QByteArray,
                          s_css,
                          ("<style type=\"text/css\">\n"
                           "\thtml { background-color: "
                           + qGuiApp->palette().color(QPalette::Active, QPalette::Base).name().toLatin1()
                           + "; }\n"
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
                             "\tli { padding-left: 12px; padding-bottom: 4px; list-style-position: inside; }\n"
                             "\t.exp { color: #000000 }\n"
                             "\ta { color: #0000ff }\n"
                             "\ta:link {text-decoration:none;}\n"
                             "\ta:visited {text-decoration:none;}\n"
                             "\ta:hover {text-decoration:underline;}\n"
                             "\ta:active {text-decoration:underline;}\n"
                             "\tp { font-size: "
                           + QByteArray::number(QFontMetrics(QGuiApplication::font()).height())
                           + "px; }\n"
                             "</style>\n"))

ConsoleModel::ConsoleModel(QObject *parent)
    : QObject(parent)
{
}

bool ConsoleModel::addOperation(const QString &input)
{
    return addOperation(Analitza::Expression(input), input);
}

bool ConsoleModel::addOperation(const Analitza::Expression &e, const QString &input)
{
    Analitza::Expression res;

    a.setExpression(e);
    if (a.isCorrect()) {
        if (m_mode == ConsoleModel::Evaluation) {
            res = a.evaluate();
        } else {
            res = a.calculate();
        }
    }

    if (a.isCorrect()) {
        a.insertVariable(u"ans"_s, res);
        m_script += e; // Script won't have the errors
        Q_EMIT operationSuccessful(e, res);

        const auto result = res.toHtml();
        addMessage(QStringLiteral("<a title='%1' href='kalgebra:/query?id=copy&func=%2'><span class='exp'>%3</span></a><br />= <a title='kalgebra:%1' "
                                  "href='kalgebra:/query?id=copy&func=%4'><span class='result'>%5</span></a>")
                       .arg(i18n("Paste to Input"), e.toString(), e.toHtml(), res.toString(), result),
                   e,
                   res);
    } else {
        addMessage(i18n("<ul class='error'>Error: <b>%1</b><li>%2</li></ul>", input.toHtmlEscaped(), a.errors().join(u"</li>\n<li>"_s)), {}, {});
    }

    return a.isCorrect();
}

bool ConsoleModel::loadScript(const QUrl &path)
{
    Q_ASSERT(!path.isEmpty() && path.isLocalFile());

    // FIXME: We have expression-only script support
    bool correct = false;
    QFile file(path.toLocalFile());

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);

        a.importScript(&stream);
        correct = a.isCorrect();
    }

    if (correct)
        addMessage(i18n("Imported: %1", path.toDisplayString()), {}, {});
    else
        addMessage(i18n("<ul class='error'>Error: Could not load %1. <br /> %2</ul>", path.toDisplayString(), a.errors().join(u"<br/>"_s)), {}, {});

    return correct;
}

bool ConsoleModel::saveScript(const QUrl &savePath)
{
    Q_ASSERT(!savePath.isEmpty());

    QFile file(savePath.toLocalFile());
    bool correct = file.open(QIODevice::WriteOnly | QIODevice::Text);

    if (correct) {
        QTextStream out(&file);
        for (const Analitza::Expression &exp : std::as_const(m_script)) {
            out << exp.toString() << QLatin1Char('\n');
        }
    }

    return correct;
}

void ConsoleModel::setMode(ConsoleMode mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        Q_EMIT modeChanged(mode);
    }
}

void ConsoleModel::setVariables(const QSharedPointer<Analitza::Variables> &vars)
{
    a.setVariables(vars);
}

void ConsoleModel::addMessage(const QString &msg, const Analitza::Expression &operation, const Analitza::Expression &result)
{
    m_htmlLog += msg.toUtf8();
    Q_EMIT updateView();
    Q_EMIT message(msg, operation, result);
}

bool ConsoleModel::saveLog(const QUrl &savePath) const
{
    Q_ASSERT(savePath.isLocalFile());
    // FIXME: We have to choose between txt and html
    QFile file(savePath.toLocalFile());
    bool correct = file.open(QIODevice::WriteOnly | QIODevice::Text);

    if (correct) {
        QTextStream out(&file);
        out << "<html>\n<head>" << *s_css << "</head>" << QLatin1Char('\n');
        out << "<body>" << QLatin1Char('\n');
        for (const QByteArray &entry : std::as_const(m_htmlLog)) {
            out << "<p>" << entry << "</p>" << QLatin1Char('\n');
        }
        out << "</body>\n</html>" << QLatin1Char('\n');
    }

    return correct;
}

void ConsoleModel::clear()
{
    m_script.clear();
    m_htmlLog.clear();
}

QByteArray ConsoleModel::css() const
{
    return *s_css;
}

QString ConsoleModel::readContent(const QUrl &url)
{
    return QUrlQuery(url).queryItemValue(u"func"_s);
}

#include "moc_consolemodel.cpp"
