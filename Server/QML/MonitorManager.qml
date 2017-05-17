import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0

Item {
    id: id_monitorManager
    property real dpScale:  1.5
    readonly property real dp: Math.max(Screen.pixelDensity * 25.4 / 160 * dpScale, 1)

    // functions
    function showClientViewport(roomID, clientGuid, imgData, imgWidth, imgHeight, imgFormat) {

    }

    Rectangle {
        id: id_monitorTools
        anchors { left: parent.left; right: parent.right; top: parent.top; bottomMargin: 5 * dp }
        height: 20 * dp
        //spacing: 30 * dp
        color: Qt.rgba(0, 0, 0, 0)

        Text {
            id: id_monitorManagerLabel
            anchors { left: parent.left; verticalCenter: parent.verticalCenter }
            color: "white"
            text: qsTr("监控玩家视角:")
        }

        CustomSwitchButton {
            id: id_flatFlowSwitch
            dp: id_monitorManager.dp
            anchors { left: id_monitorManagerLabel.right; verticalCenter: parent.verticalCenter; leftMargin: 30*dp }
            leftText: qsTr("内嵌")
            rightText: qsTr("浮动")
        }

        CustomCheckBox {
            id: id_windowTopSwitch
            dp: id_monitorManager.dp
            anchors { left: id_flatFlowSwitch.right; verticalCenter: parent.verticalCenter; leftMargin: 30*dp }
            height: 20 * id_monitorManager.dp
            textColor: "white"
            text: qsTr("浮动窗口置顶")
        }

        CustomComboBox {
            dp: id_monitorManager.dp
            anchors { left: id_windowTopSwitch.right; verticalCenter: parent.verticalCenter; leftMargin: 30*dp }
            width: 80 * id_monitorManager.dp
            model: ["一般画质", "清晰画质", "高清画质"]
        }
    }

    Rectangle {
        id: id_monitorArea
        anchors { left: parent.left; right: parent.right; top: id_monitorTools.bottom; bottom: parent.bottom; topMargin: 5 * dp }
        color: "transparent"
        border { width: 1; color: Qt.rgba(0, 0.5, 0.8, 1) }
    }
}
