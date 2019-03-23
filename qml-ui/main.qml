import QtQuick 2.9
import QtQuick.Controls 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4 as Controls14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Styles 1.4 as Styles
import QtMultimedia 5.8

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: qsTr("Capture Demo")
    Popup{ id:pwin}

    menuBar: MenuBar {
        Menu {
            title: "Edit"
            MenuItem { text: "Setting"
            onClicked: pwin.show();
            }
        }
    }

    Controls14.SplitView{
        anchors.fill:parent;
        orientation: Qt.Horizontal;
        Rectangle{
            id:rect1;
            Layout.fillWidth: true;
            Layout.minimumWidth: 560
            MediaPlayer {
                id: mediaPlayer
                source: "file:///I:/怒晴湘西02.mp4"
                autoPlay: true
            }

            VideoOutput {
                id: video
                anchors.fill: parent
                source: mediaPlayer
            }
            MouseArea {
                id: playArea
                anchors.fill: parent
                property int status: 1
                anchors.bottomMargin: 0
                anchors.leftMargin: 0
                anchors.topMargin: 0
                anchors.rightMargin: -65
                onPressed:  {
                    if(status == 1)
                    {
                        status = 0
                        mediaPlayer.pause()
                    }
                    else
                    {
                        status = 1
                        mediaPlayer.play()
                    }
                }

            }
            Rectangle {
                id: progressBar
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 0
                height: 10
                color: "lightGray"

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: mediaPlayer.duration>0?parent.width*mediaPlayer.position/mediaPlayer.duration:0
                    color: "darkGreen"
                }

                MouseArea {
                    property int pos
                    anchors.fill: parent

                    onClicked: {
                    if (mediaPlayer.seekable)
                        pos = mediaPlayer.duration * mouse.x/width
                        mediaPlayer.seek(pos)
                    }
                }
            }
        }
        Rectangle{
            id:rect2;
            x: 599
            width: 200;
            //color:"blue";
            ComboBox {
                id:box1;
                y: 22
                anchors.horizontalCenterOffset: 0
                model: ["Usb camera", "video capture"]
                //y: 50;
                anchors.horizontalCenter:  parent.horizontalCenter;
                onAccepted: {
                    console.log("input: " + currentText)
                }
            }
            ComboBox {
                    id:box2;
                    y: 87
                    anchors.horizontalCenterOffset: 0

                    //anchors.Top:box1.bottom;
                    model: ["motor1", "motor2", "motor3"]
                    //y: 50;
                    anchors.horizontalCenter:  parent.horizontalCenter;
                    onAccepted: {
                        console.log("input: " + currentText)
                    }
            }
            FlatButton {
                id: rotate_right;
                x: 102
                y: 400
                width: 64
                height: 37
                iconSource: "icons/ic_rotate_right.png";
                //text: qsTr("Search Internet");
                font.pointSize: 14;
                onClicked: {
                    console.log("right button clicked")
                    MySerialPort.motor_move(box2.currentIndex+1, Number(tfiled.text), 0, speedCombox.currentIndex+1)
                }
            }

            Text {
                id: element
                x: 54
                y: 359
                text: qsTr("Angle:")
                font.pixelSize: 12
            }
            TextField {
                id: tfiled
                x: 96
                y: 351
                width: 70
                height: 30
            }
            FlatButton {
                id: rotate_left;
                x: 40
                y: 396
                width: 62
                height: 37
                iconSource: "icons/ic_rotate_left.png";
                //text: qsTr("Search Internet");
                font.pointSize: 14;
                onClicked: {
                    console.log("left button clicked")
                    MySerialPort.motor_move(box2.currentIndex+1, Number(tfiled.text), 1, speedCombox.currentIndex+1)
                }
            }

            ComboBox {
                id: comCombox
                x: 4
                y: 147
                width: 140
                height: 44
                model:MySerialPort.portName
                anchors.horizontalCenterOffset: 0
                anchors.horizontalCenter: parent.horizontalCenter
            }

            ComboBox {
                id: speedCombox
                x: -2
                y: 285
                width: 140
                height: 36
                model: ["1", "4","8"]
                anchors.horizontalCenterOffset: 0
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Button{
                //objectName: "operateBtn";
                id:operateBtn;
                x: 30
                y: 222
                width:140
                height: 35;
                //anchors.verticalCenter: parent.verticalCenter;
                text:"Open";
                anchors.horizontalCenterOffset: 0
                anchors.horizontalCenter: parent.horizontalCenter
                //anchors.verticalCenterOffset: 65
                onClicked:MySerialPort.serialPortOperate(comCombox.currentIndex,speedCombox.currentIndex,0,0,0);
            }

            FlatButton {
                id: run_left
                x: 22
                y: 455
                width: 55
                height: 32
                iconSource: "icons/ic_backward.png";
                font.pointSize: 14;
                onClicked: {
                    console.log("left button clicked")
                    MySerialPort.motor_run(box2.currentIndex, 1, speedCombox.currentIndex)
                }
            }
            FlatButton {
                id: run_right
                x: 78
                y: 454
                width: 50
                height: 33
                iconSource: "icons/ic_forward.png";
                onClicked: {
                    console.log("left button clicked")
                    MySerialPort.motor_run(box2.currentIndex, 0, speedCombox.currentIndex)
                }
            }

            FlatButton {
                id: run_stop
                x: 133
                y: 453
                width: 52
                height: 40
                iconSource: "icons/ic_media_pause.png";
                font.pointSize: 14;
                onClicked: {
                    console.log("left button clicked")
                    MySerialPort.motor_run(box2.currentIndex, 2, speedCombox.currentIndex)
                }
            }
        }
        Connections{
              target:MySerialPort;
              onLinkStatusChanged:{
                  if(MySerialPort.linkStatus==true){
                      operateBtn.text="Close";
                      comCombox.enabled=false;
                      speedCombox.enabled=false;
                      //stopCombox.enabled=false;
                      //dataCombox.enabled=false;
                      //parityCombox.enabled=false;
                  }
                  else{
                      operateBtn.text="Open";
                      comCombox.enabled=true;
                      speedCombox.enabled=true;
                      //stopCombox.enabled=true;
                      //dataCombox.enabled=true;
                      //parityCombox.enabled=true;
                  }
              }
        }
    }
}















































/*##^## Designer {
    D{i:3;anchors_width:200}
}
 ##^##*/
