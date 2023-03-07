import QtQuick 2.0

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.0
import QtQuick.Controls.Material 2.12

// Base element for QOpenHD settings.
// NOTE: QOpenHD, not OpenHD/Mavlink settings !
// It does not define the element that is actually then used to edit the QT setting -
// See any implementation for an example how to use.
// This element could be a Switch, a SpinBox, ...
Rectangle {
    width: parent.width
    height: rowHeight
    // Item needs to be placed in a Column, in which case columns are colored alternating white - light blue
    // This increases readability
    color: (Positioner.index % 2 == 0) ? "#8cbfd7f3" : "#00000000"

    // Set this to the setting(s) short description.
    // This value is always shown inside a text view on the left
    property string m_short_description: "PLACEHOLDER"

    // Optionally - set this to the setting(s) long descrition.
    // If a long description is set, a "info" icon is shown right next
    // to the setting(s) short description, and clicking it opens up a popup
    // with the long description.
    property string  m_long_description: "none"

    function has_valid_long_description(){
        return m_long_description !== "none"
    }

    Text {
        id: description
        text: qsTr(m_short_description)
        font.weight: Font.Bold
        font.pixelSize: 13
        anchors.leftMargin: 8
        verticalAlignment: Text.AlignVCenter
        anchors.verticalCenter: parent.verticalCenter
        width: 224
        height: elementHeight
        anchors.left: parent.left
    }

    Button {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: description.right
        text: "INFO"
        Material.background:Material.LightBlue
        visible: has_valid_long_description()
        onClicked: {
            _messageBoxInstance.set_text_and_show(m_long_description)
        }
    }

}
