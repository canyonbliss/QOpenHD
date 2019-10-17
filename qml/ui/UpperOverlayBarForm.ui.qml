import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.13
import QtGraphicalEffects 1.13

import OpenHD 1.0

import Qt.labs.settings 1.0

import "./widgets";

Rectangle {
    id: toolBar
    property alias settingsButton: settingsButton
    property alias settingsButtonMouseArea: settingsButtonMouseArea

    width: 800

    // fixme: shouldnt exclusively depend on mavlink
    color: MavlinkTelemetry.armed ? "#aeff3333" : "#8f000000"

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }

    z: 2.0

    height: 48

    Image {
        id: settingsButton
        x: 8
        y: 8
        width: 48
        height: 48
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
        z: 2.2

        source: "../ic.png"
        anchors.leftMargin: 8
        anchors.topMargin: 8
        MouseArea {
            id: settingsButtonMouseArea
            anchors.fill: parent

        }
    }
}

/*##^##
Designer {
    D{i:4;anchors_x:0}D{i:5;anchors_width:24;anchors_x:0}D{i:3;anchors_height:24}D{i:7;anchors_width:24;anchors_x:0}
D{i:8;anchors_width:34;anchors_x:0;anchors_y:0}D{i:9;anchors_height:20;anchors_width:34}
D{i:6;anchors_x:0}D{i:11;anchors_width:34;anchors_x:0;anchors_y:0}D{i:12;anchors_height:20;anchors_width:34;anchors_x:0;anchors_y:11}
D{i:15;anchors_width:34;anchors_x:0;anchors_y:11}D{i:16;anchors_width:32}D{i:17;anchors_width:38}
D{i:19;anchors_width:32}D{i:20;anchors_width:38}
}
##^##*/
