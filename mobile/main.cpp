// SPDX-FileCopyrightText: 2010 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QGuiApplication>
#ifdef Q_OS_ANDROID
#include <QQuickStyle>
#else
#include <QApplication>
#endif

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>

#include "kalgebra_version.h"
#include "kalgebramobile.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    QQuickStyle::setStyle("Material");
#endif
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    KLocalizedString::setApplicationDomain("kalgebramobile");
    KAboutData about(QStringLiteral("kalgebramobile"),
                     QStringLiteral("KAlgebra"),
                     QStringLiteral(KALGEBRA_VERSION_STRING),
                     i18n("A portable calculator"),
                     KAboutLicense::GPL,
                     i18n("(C) 2006-2023 Aleix Pol i Gonzalez"));
    about.addAuthor(i18n("Aleix Pol i Gonzalez"), i18nc("@info:credit", "Maintainer"), QStringLiteral("aleixpol@kde.org"));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    KAboutData::setApplicationData(about);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kalgebra")));

    {
        QCommandLineParser parser;
        about.setupCommandLine(&parser);
        parser.process(app);
        about.processCommandLine(&parser);
    }

    KAlgebraMobile widget;

    QQmlApplicationEngine engine;

    qmlRegisterSingletonInstance("org.kde.kalgebra.mobile", 1, 0, "App", &widget);
    qmlRegisterSingletonType("org.kde.kalgebra.mobile", 1, 0, "About", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
        return engine->toScriptValue(KAboutData::applicationData());
    });
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
