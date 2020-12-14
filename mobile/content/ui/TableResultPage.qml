/**
 * SPDX-FileCopyrightText: Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import org.kde.kirigami 2.14 as Kirigami

Kirigami.ScrollablePage {
    title: i18n("Results")

    required property var results

    ListView {
        currentIndex: -1
        model: results
        delegate: Kirigami.BasicListItem {
            label: model.element
        }
    }
}
