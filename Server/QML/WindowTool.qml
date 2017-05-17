import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0

Rectangle {
    id: id_windowTool
    property real dpScale:  1.5
    readonly property real dp: Math.max(Screen.pixelDensity * 25.4 / 160 * dpScale, 1)

    // define signals
//    signal showServerIp(list ipList)

    // functions
    function showServerIp(ipList) {
        for (var index in ipList) {
            //id_serverIpContainer.model += [ipList[index]];
            ipModel.append({"ip": ipList[index]});
        }
    }

    function makeText(num) {
        if (num > 9) {
            return qsTr("%1").arg(num);
        }
        else {
            return qsTr("0%1").arg(num);
        }
    }

    //color: Qt.rgba(0, 122, 204, 0.58)
    color: Qt.rgba(0, 0.5, 0.8, 0.6);

    // server state label
    Text {
        id: id_serverStateText
        anchors { left: parent.left; verticalCenter: parent.verticalCenter; margins: 5 * id_windowTool.dp }
        font { pointSize: 10 }
        color: "white"
        text: qsTr("服务器监听状态")
    }

    // server state breathing light
    Canvas {
        id: id_serverStateImg
        anchors { left: id_serverStateText.right; verticalCenter: parent.verticalCenter; margins: 3 * id_windowTool.dp }
        width: 11 * id_windowTool.dp;
        height: width
        onPaint: {
            var ctx = getContext("2d");
            ctx.strokeStyle = "green"
            ctx.fillStyle = ctx.strokeStyle;
            ctx.beginPath();
            ctx.ellipse(0, 0, id_serverStateImg.width, id_serverStateImg.height);
            ctx.fill();
            ctx.stroke();
        }
        Timer {
            interval: 1000
            repeat: true
            running: true
            onTriggered: {
                parent.visible = !parent.visible;
            }
        }
    }

    // server running time
    Text {
        id: id_serverRunningTime
        anchors { left: id_serverStateImg.right; verticalCenter: parent.verticalCenter; margins: 50 * id_windowTool.dp }
        color: id_serverStateText.color
        font: id_serverStateText.font
        text: qsTr("服务器运行时间: 00年00月00天00时00分00秒")
        Timer {
            interval: 1000
            repeat: true
            running: true
            property int timeYear: 0
            property int timeMonth: 0
            property int timeDay: 0
            property int timeHour: 0
            property int timeMin: 0
            property int timeSec: 0
            onTriggered: {
                timeSec += 1;
                if (timeSec > 59) {
                    timeMin += 1;
                    timeSec = 0;
                    if (timeMin > 59) {
                        timeHour += 1;
                        timeMin = 0;
                        if (timeHour > 23)
                        {
                            timeDay += 1;
                            timeHour = 0;
                            if (timeDay > 29) {
                                timeMonth += 1;
                                timeDay = 0;
                                if (timeMonth > 11) {
                                    timeYear += 1;
                                    timeMonth = 0;
                                }
                            }
                        }
                    }
                }
                //parent.text = qsTr("服务器运行时间: %L1年%2月%3天%4时%5分%6秒").arg(timeYear).arg(timeMonth).arg(timeDay).arg(timeHour).arg(timeMin).arg(timeSec)
                parent.text = qsTr("服务器运行时间: %L1年%2月%3天%4时%5分%6秒").arg(makeText(timeYear)).arg(makeText(timeMonth)).arg(makeText(timeDay)).arg(makeText(timeHour)).arg(makeText(timeMin)).arg(makeText(timeSec))
            }
        }
    }

    // server ip label
    Text {
        id: id_serverIp
        anchors { left: id_serverRunningTime.right; verticalCenter: parent.verticalCenter; margins: 50 * id_windowTool.dp }
        color: id_serverStateText.color
        font: id_serverStateText.font
        text: qsTr("服务器 IP: ")
    }

    // server ip comtainer
    CustomComboBox {
        id: id_serverIpContainer
        dp: id_windowTool.dp
        anchors { left: id_serverIp.right; verticalCenter: parent.verticalCenter }
        width: 110 * id_windowTool.dp
        height: 20 * id_windowTool.dp
        model: ipModel
        currentIndex: ipModel.count-1

        ListModel {
            id: ipModel
        }
    }

    Button {
        id: id_openServerCacheDir
        anchors { right: id_openServerLog.left; verticalCenter: parent.verticalCenter; margins: 5 * id_windowTool.dp }
        height: 20 * id_windowTool.dp;
        text: qsTr("打开缓存目录")
    }

    Button {
         id: id_openServerLog
        anchors { right: parent.right; verticalCenter: parent.verticalCenter; margins: 5 * id_windowTool.dp }
        height: 20 * id_windowTool.dp;
        text: qsTr("查看日志")
    }
}
