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

#include <QGuiApplication>

#ifdef KDECOMPONENTS
#include <KLocalizedContext>
#endif

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QIcon>

#include "kalgebramobile.h"
#include "pluginsmodel.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
#ifdef __ANDROID__
    qputenv("QT_QUICK_CONTROLS_STYLE", "material");
#endif
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//     KAboutData about("kalgebra", 0, ki18n(("KAlgebra Mobile")), "0.10", ki18n("A portable calculator"),
//              KAboutData::License_GPL, ki18n("(C) 2006-2010 Aleix Pol Gonzalez"));
//     about.addAuthor( ki18n("Aleix Pol Gonzalez"), KLocalizedString(), "aleixpol@kde.org" );
//     KCmdLineArgs::init(argc, argv, &about);
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kalgebramobile"));
//     app.setWindowIcon(QIcon::fromTheme("kalgebra"));
    
    KAlgebraMobile widget;
    
    QString main = PluginsModel::pluginsDirectoryPath()+"/widgets/KAlgebraMobile.qml";

    QDir dir = QFileInfo(main).dir();
    dir.cdUp();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("app"), &widget);
    engine.addImportPath(dir.path());

#ifdef KDECOMPONENTS
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
#endif

    engine.load(QUrl::fromLocalFile(main));
    return app.exec();
}
