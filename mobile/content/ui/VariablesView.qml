// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.0
import org.kde.analitza 1.0
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kalgebra.mobile 1.0
import QtQuick.Controls 2.5 as QQC2


Kirigami.ScrollablePage {
    title: i18n("Variables")
    Kirigami.CardsListView {
        anchors.fill: parent
        model: VariablesModel { variables: App.variables }
        currentIndex: -1
        delegate: Kirigami.Card {
            contentItem: QQC2.Label { text: model.whatsThis }
        }
    }
}
