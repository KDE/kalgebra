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
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
    id: page
    ColumnLayout {
        anchors.margins: 20*page.dp
        anchors.fill: parent
        spacing: 20*page.dp
        
        Image {
            id: icon
            source: "qrc:/kalgebra.svgz"
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
            sourceSize: Qt.size(parent.width/4, parent.width/4)

            MouseArea {
                anchors.fill: parent
                onClicked: Qt.openUrlExternally("http://edu.kde.org/applications/mathematics/kalgebra")
            }
        }
        
        Label {
            Layout.fillWidth: true
            
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify
            text: i18n("KAlgebra is brought to you by the lovely community of <a href='http://kde.org'>KDE</a> and <a href='http://edu.kde.org/'>KDE Edu</a> from a Free Software environment.")
            onLinkActivated: Qt.openUrlExternally(link)
        }
        
        Label {
            Layout.fillWidth: true
            
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify
            text: i18n("In case you want to learn more about KAlgebra, you can find more information <a href='https://edu.kde.org/applications/mathematics/kalgebra/'>in the official site</a> and in the <a href='https://userbase.kde.org/KAlgebra'>users wiki</a>.<br/>If you have any problem with your software, please report it to <a href='https://bugs.kde.org'>our bug tracker</a>.")
            onLinkActivated: Qt.openUrlExternally(link)
        }
        
        Label {
            Layout.fillWidth: true
            
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignJustify
            text: "Aleix Pol Gonzalez &lt;<a href='mailto:aleixpol@kde.org'>aleixpol@kde.org</a>&gt;"
            onLinkActivated: Qt.openUrlExternally(link)
        }
        
        Image {
            source: "qrc:/kde-edu-logo.png"
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
            Layout.maximumHeight: icon.height
            
            MouseArea {
                anchors.fill: parent
                onClicked: Qt.openUrlExternally("http://edu.kde.org")
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
