import QtQuick 2.9
import QtQuick.Controls 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4 as Controls14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Styles 1.4 as Styles
import QtMultimedia 5.8

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    Controls14.SplitView{
        anchors.fill:parent;
        orientation: Qt.Horizontal;
        Rectangle{
            id:rect1;
            Layout.fillWidth: true;
            Layout.minimumWidth: 440
            MediaPlayer {
                id: mediaPlayer
                source: "file:///H:/怒晴湘西02.mp4"
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
                id: baudCombox
                x: -2
                y: 212
                width: 140
                height: 44
                model: ["4800", "9600","115200","230400","460800"]
                anchors.horizontalCenterOffset: 0
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Button{
                //objectName: "operateBtn";
                id:operateBtn;
                x: 30
                width:140
                height: 35;
                anchors.verticalCenter: parent.verticalCenter;
                text:"Open";
                anchors.verticalCenterOffset: 62;
                onClicked:MySerialPort.serialPortOperate(comCombox.currentIndex,baudCombox.currentIndex,0,0,0);
            }
        }
        Connections{
              target:MySerialPort;
              onLinkStatusChanged:{
                  if(MySerialPort.linkStatus==true){
                      operateBtn.text="Close";
                      comCombox.enabled=false;
                      baudCombox.enabled=false;
                      //stopCombox.enabled=false;
                      //dataCombox.enabled=false;
                      //parityCombox.enabled=false;
                  }
                  else{
                      operateBtn.text="Open";
                      comCombox.enabled=true;
                      baudCombox.enabled=true;
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
