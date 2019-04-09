import QtQuick 2.9
import QtQuick.Controls 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4 as Controls14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Styles 1.4 as Styles
import org.freedesktop.gstreamer.GLVideoItem 1.0

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
            color: "black"
            Item {
                anchors.fill: parent
                GstGLVideoItem {
                    id: videoItem
                    objectName: "videoItem"
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height
                }
            }

        }

        Rectangle{
            id:rect2;
            x: 599
            width: 200;
            ComboBox {
                id:videoinputsrc
                objectName: "video-input-src"
                y: 22
                anchors.horizontalCenterOffset: 0
                model: ["HDMI 4K", "SDI 4K", "MISC"]
                anchors.horizontalCenter:  parent.horizontalCenter
                signal inputSrc(string src)
                function trigger() {
                    if (currentIndex == 0)
                        inputSrc("/dev/video0")
                    else if (currentIndex == 1)
                        inputSrc("/dev/video1")
                    else
                        inputSrc("/dev/video2")
                }
                onActivated: trigger()
            }
        }
    }
}
