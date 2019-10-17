import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtGraphicalEffects 1.13
import Qt.labs.settings 1.0

import OpenHD 1.0

BaseWidget {
    id: airBatteryWidget
    width: 96
    height: 48

    widgetIdentifier: "air_battery_widget"

    defaultAlignment: 3
    defaultXOffset: 0
    defaultYOffset: 0
    defaultHCenter: false
    defaultVCenter: false

    hasWidgetDetail: true
    widgetDetailComponent: Column {
        Item {
            width: parent.width
            height: 24
            Text { text: "Voltage:";  color: "white"; font.bold: true; anchors.left: parent.left }
            Text { text: MavlinkTelemetry.battery_voltage; color: "white"; font.bold: true; anchors.right: parent.right }
        }
        Item {
            width: parent.width
            height: 24
            Text { text: "Current:";  color: "white"; font.bold: true; anchors.left: parent.left }
            Text { text: MavlinkTelemetry.battery_current; color: "white"; font.bold: true; anchors.right: parent.right }
        }
    }

    Item {
        id: widgetInner

        anchors.fill: parent
        Text {
            id: battery_percent
            y: 0
            width: 48
            height: 24
            color: "#ffffff"
            text: MavlinkTelemetry.battery_percent
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: batteryGauge.right
            anchors.leftMargin: 0
            clip: true
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 14
        }

        Text {
            id: batteryGauge
            y: 8
            width: 36
            height: 48
            color: MavlinkTelemetry.battery_voltage_raw < 11.2 ? (MavlinkTelemetry.battery_voltage_raw < 10.9 ? "#ff0000" : "#fbfd15") : "#ffffff"
            text: MavlinkTelemetry.battery_gauge
            anchors.left: parent.left
            anchors.leftMargin: 12
            fontSizeMode: Text.VerticalFit
            anchors.verticalCenter: parent.verticalCenter
            clip: true
            elide: Text.ElideMiddle
            font.family: "Material Design Icons"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 36
        }
    }
}
