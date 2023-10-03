// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import org.kde.analitza
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kalgebra.mobile

Kirigami.ScrollablePage {
    title: i18n("Variables")

    ListView {
        model: VariablesModel { variables: App.variables }
        currentIndex: -1
        delegate: Delegates.RoundedItemDelegate {
            required property int index
            required property string whatsThis

            text: whatsThis
        }
    }
}
