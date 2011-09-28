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

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include "kalgebramobile.h"
#include <QDeclarativeView>
#include <KStandardDirs>
#include <QDebug>
#include <QDeclarativeEngine>
#include <qdeclarativecontext.h>

int main(int argc, char *argv[])
{
// 	KAboutData about("kalgebra", 0, ki18n(("KAlgebra Mobile")), "0.10", ki18n("A portable calculator"),
// 			 KAboutData::License_GPL, ki18n("(C) 2006-2010 Aleix Pol Gonzalez"));
// 	about.addAuthor( ki18n("Aleix Pol Gonzalez"), KLocalizedString(), "aleixpol@kde.org" );
// 	KCmdLineArgs::init(argc, argv, &about);
	QApplication app(argc, argv);
	
	KAlgebraMobile widget;
	
	QDeclarativeView view;
	view.setResizeMode(QDeclarativeView::SizeRootObjectToView);
	view.engine()->rootContext()->setContextProperty("app", &widget);
	
	QString main = KStandardDirs::locate("data", "kalgebra/plugins/widgets/KAlgebraMobile.qml");
//     QString main = KStandardDirs::locate("data", "kalgebra/plugins/Tables.qml");
	view.setSource(main);
	view.show();
	
	return app.exec();
}
