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

#include <QFile>
#include <QUrl>
#include <KLocalizedString>

bool ConsoleModel::addOperation(const Analitza::Expression& e, const QString& input)
{
    Analitza::Expression res;

    a.setExpression(e);
    if(a.isCorrect()) {
        if (m_mode==ConsoleModel::Evaluation) {
            res=a.evaluate();
        } else {
            res=a.calculate();
        }
    }

    QString result, newEntry;
    if(a.isCorrect()) {
        result = res.toHtml();

        a.insertVariable(QStringLiteral("ans"), res);
        m_script += e; //Script won't have the errors
        newEntry = QString("<a title='%1' href='kalgebra:/query?id=copy&func=%2'><span class='exp'>%3</span></a><br />=<a title='kalgebra:%1' href='/query?id=copy&func=%4'><span class='result'>%5</span>")
            .arg(i18n("Paste to Input")).arg(e.toString()).arg(e.toHtml()).arg(res.toString()).arg(result);
    } else {
        errorMessage(i18n("<ul class='error'>Error: <b>%1</b><li>%2</li></ul>", input.toHtmlEscaped(), a.errors().join(QStringLiteral("</li>\n<li>"))));
    }

    updateView(newEntry);

    return a.isCorrect();
}

bool ConsoleModel::loadScript(const QString& path)
{
    Q_ASSERT(!path.isEmpty());

    //FIXME: We have expression-only script support
    bool correct=false;
    QFile file(path);

    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);

        a.importScript(&stream);
        correct=a.isCorrect();
    }

    if(!correct) {
        Q_EMIT errorMessage(i18n("<ul class='error'>Error: Could not load %1. <br /> %2</ul>", path, a.errors().join(QStringLiteral("<br/>"))));
        updateView(QString());
    }
    else
        updateView(i18n("Imported: %1", path));

    return correct;
}

bool ConsoleModel::saveScript(const QString& savePath)
{
    Q_ASSERT(!savePath.isEmpty());

    QFile file(savePath);
    bool correct=file.open(QIODevice::WriteOnly | QIODevice::Text);

    if(correct) {
        QTextStream out(&file);
        foreach(const Analitza::Expression& exp, m_script)
            out << exp.toString() << endl;
    }

    return correct;
}
