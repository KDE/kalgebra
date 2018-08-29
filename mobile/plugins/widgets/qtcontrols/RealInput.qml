import QtQuick.Controls 2.0
import QtQuick 2.0

TextField
{
    id: input

    readonly property real realValue: Number.fromLocaleString(locale, text)
    validator: DoubleValidator {}
}
