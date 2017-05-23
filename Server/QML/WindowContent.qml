import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0

import QtCPlusPlus.UdpTransmission 2.0

Rectangle {
    id: id_windowContent
    color: "transparent"
    property real dpScale:  1.5
    readonly property real dp: Math.max(Screen.pixelDensity * 25.4 / 160 * dpScale, 1)
    readonly property real margin: 5 * dp

    // server state and tools
    WindowTool {
        id: id_windowTool
        anchors { left: parent.left; right: parent.right; leftMargin: id_windowContent.margin; rightMargin: id_windowContent.margin }
        height: 30 * id_windowContent.dp
    }

    // server console message label
    Text {
        id: id_consoleMsgLabel
        anchors { left: parent.left; top: id_windowTool.bottom; margins: id_windowContent.margin }
        color: "white"
        text: qsTr("服务器消息:")
    }

    // server console message container
    Rectangle {
        id: id_consoleMsgContainer
        anchors { left: parent.left; top: id_consoleMsgLabel.bottom; bottom: id_monitorManager.top; margins: id_windowContent.margin }
        width: parent.width *4 / 6
        color: "transparent"
        border { width: 1; color: Qt.rgba(0, 0.5, 0.8, 1) }

        Flickable {
            anchors { fill: parent }

            TextArea.flickable: TextArea{
                id: id_consoleMsgTextArea
                color: "white"
                selectByMouse: true
                readOnly: true
            }

            ScrollBar.vertical:  ScrollBar {
                active: id_consoleMsgTextArea.contentHeight > parent.height

                onActiveChanged: {
                    active = true;
                }
            }
        }
    }

    // rooms manager
    RoomManager {
        id: id_roomManager
        anchors { left: id_consoleMsgContainer.right; right: parent.right; top: id_windowTool.bottom; bottom: id_monitorManager.top; margins: id_windowContent.margin }
        onSglSwitchMonitorClientViewport: {
            id_udpTransmission.setClientViewportState(onOff, roomID, 0);
        }
    }

    // monitor manager
    MonitorManager {
        id: id_monitorManager
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: id_windowContent.margin }
        height: parent.height * 2 /5
    }

    // c++ net transmission
    UdpTransmission{
        id: id_udpTransmission
        Component.onCompleted: {
            // bind signal slot
            id_udpTransmission.sglPrintString.connect(showConsoleMsg);

            id_udpTransmission.sglShowServerIp.connect(id_windowTool.showServerIp);

            id_udpTransmission.sglCreateNewRoom.connect(id_roomManager.createNewRoom);
            id_udpTransmission.sglJoinRoom.connect(id_roomManager.joinRoom);
            id_udpTransmission.sglLeaveRoom.connect(id_roomManager.leaveRoom);

            id_udpTransmission.sglShowClientViewport.connect(id_monitorManager.showClientViewport);

            //id_udpTransmission.sglUpdateUsername.connect()
            //void sglUpdateUsername(QString roomID, QString clientGuid, QString newUsername);
        }
    }

    // start server
    Component.onCompleted: {
        id_udpTransmission.start();
    }

    // show c++ console message
    function showConsoleMsg(msg) {
        id_consoleMsgTextArea.append(msg);
    }
}
