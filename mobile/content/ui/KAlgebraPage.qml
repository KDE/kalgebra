// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import org.kde.kirigami 2.0
import QtQuick 2.1

Page
{
    readonly property real dp: Units.devicePixelRatio
    title: "KAlgebra"
    default property alias contents: item.data

    Item {
        id: item
        anchors.fill: parent
    }
}
