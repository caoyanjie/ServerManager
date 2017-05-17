import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0

Item {
    id: id_roomManager
    property real dpScale:  1.5
    readonly property real dp: Math.max(Screen.pixelDensity * 25.4 / 160 * dpScale, 1)

    // functions
    function createNewRoom(clientGuid, clientName, roomPasswd) {
        id_roomComponent.createObject(
                    id_roomsContainer,
                    {
                        "width": 80*id_roomManager.dp,
                        "height": 60*id_roomManager.dp,
                        "roomName": clientName,
                        "roomID": clientGuid,
                        "roomHost": clientName,
                        "roomPasswd": roomPasswd,
                        "clientCount": 1
                    }
                    );
    }

    function joinRoom(roomID, clientGuid, clientRoom) {
        for (var i=0; i<id_roomsContainer.children.length; ++i) {
            var room = id_roomsContainer.children[i];
            if (room.roomID === roomID) {
                room.clientCount += 1;
                break;
            }

        }
    }

    function leaveRoom(roomID, clientGuid, clientRoom) {
        for (var i=0; i<id_roomsContainer.children.length; ++i) {
            var room = id_roomsContainer.children[i];
            if (room.roomID === roomID) {
                room.clientCount -= 1;
                if (room.clientCount === 0) {
                    room.destroy();
                }
                break;
            }

        }
    }

    // rooms manager label
    Text {
        id: id_roomManagerLabel
        anchors { left: parent.left; top: parent.top; bottomMargin: 5 * id_roomManager.dp }
        color: "white"
        text: qsTr("房间信息:")
    }

    // rooms manager container
    Rectangle {
        //id: id_roomsContainer
        anchors { left: parent.left; right: parent.right; top: id_roomManagerLabel.bottom; bottom: parent.bottom; topMargin: 5 * id_roomManager.dp }
        color: "transparent"
        border { width: 1; color: Qt.rgba(0, 0.5, 0.8, 1) }
        Flow {
            id: id_roomsContainer
            anchors { fill: parent; margins: 10 * id_roomManager.dp }
            spacing: 10 * id_roomManager.dp
        }
    }

    Component {
        id: id_roomComponent
        Item {
            id: id_room
            property string roomName:   "null"
            property string roomID:     "null"
            property string roomHost:   "null"
            property string roomPasswd: "null"
            property int    clientCount: 0

            Image {
                id: id_roomImg
                anchors { top: parent.top; horizontalCenter: parent.horizontalCenter }
                width: parent.width / 2
                height: width
                source: "/Images/logo.png"
                MouseArea {
                    id: area_roomImg
                    anchors.fill: parent
                    hoverEnabled: true
                }
                ToolTip {
                    visible: area_roomImg.containsMouse
                    text: qsTr("创建人:   %1\n房间ID:   %2\n房间口令: %3\n房间人数: %4 人").arg(id_room.roomHost).arg(id_room.roomID).arg(id_room.roomPasswd).arg(id_room.clientCount)
                }
            }
            Text {
                id: id_roomName
                anchors { top: id_roomImg.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 5 * id_roomManager.dp }
                width: parent.width
                color: "white"
                verticalAlignment: Text.AlignTop
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WrapAnywhere
                text: id_room.roomName
            }
            CustomSwitchButton {
                id: id_monitorSwitch
                dp: id_roomManager.dp
                anchors { top: id_roomName.bottom; left: parent.left; right: parent.right; topMargin: 5 * dp }
                leftText: qsTr("关闭")
                rightText: qsTr("监控")
            }
        }
    }
}
