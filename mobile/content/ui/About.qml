/*************************************************************************************
 *  Copyright (C) 2015 by Aleix Pol <aleixpol@kde.org>                               *
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

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.5
import org.kde.analitza 1.0
import org.kde.kirigami 2.6 as Kirigami

Kirigami.AboutPage {
    aboutData: {
        "displayName": i18n("KAlgebra Mobile"),
        "productName": "kalgebramobile",
        "componentName": "kalgebramobile",
        "shortDescription": i18n("A simple scientific calculator"),
        "programIconName": "kalgebra",
        "homepage": "https://edu.kde.org/kalgebra/",
        "bugAddress": "submit@bugs.kde.org",
        "otherText": "",
        "authors": [
            {
                "name": "Aleix Pol Gonzalez",
                "task": "",
                "emailAddress": "aleixpol@kde.org",
                "webAddress": "https://proli.net",
                "ocsUsername": ""
            }
        ],
        "credits": [],
        "translators": [],
        "licenses": [
            {
                "name": "GPL v2",
                "spdx": "GPL-2.0"
            }
        ],
        "copyrightStatement": "© 2007-2019 by Aleix Pol Gonzalez",
        "desktopFileName": "org.kde.kalgebramobile"
    }
}
