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

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include "kalgebra.h"

int main(int argc, char *argv[])
{
	KAboutData about("kalgebra", 0, ki18n(("KAlgebra")), "0.8", ki18n("A calculator"),
			 KAboutData::License_GPL, ki18n("(C) 2006-2008 Aleix Pol Gonzalez"));
	about.addAuthor( ki18n("Aleix Pol Gonzalez"), KLocalizedString(), "aleixpol@gmail.com" );
	KCmdLineArgs::init(argc, argv, &about);
	KApplication app;
	
	/*if(app.isSessionRestored()) {
		RESTORE(KAlgebra);
	} else {*/
		KAlgebra* widget = new KAlgebra;
		widget->show();
	//}
	
	return app.exec();
}
