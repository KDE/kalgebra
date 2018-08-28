import QtQuick.Controls 2.0

SpinBox
{
    id: input
    property alias minimumValue: input.from
    property alias maximumValue: input.to
    property string text
    value: text

    signal accepted()
}
