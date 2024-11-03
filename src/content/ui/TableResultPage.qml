// SPDX-FileCopyrightText: Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

Kirigami.ScrollablePage {
    title: i18n("Results")

    required property var results

    ListView {
        currentIndex: -1
        model: results
        delegate: Delegates.RoundedItemDelegate {
            required property int index
            required property string element

            text: element
        }
    }
}
