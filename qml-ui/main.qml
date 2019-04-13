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
            width: 600
            Layout.fillWidth: true;
            Layout.minimumWidth: 560
            color: "black"
            Item {
                width: 600
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
            width: 200
            ComboBox {
                id:videoinputsrc
                objectName: "video-input-src"
                y: 22
                height: 35
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

            Text {
                id: fourcc
                x: 32
                y: 148
                width: 48
                height: 22
                text: qsTr("FourCC:")
                font.pixelSize: 12
            }

            Label {
                id: cclabel
                x: 87
                y: 147
                width: 79
                height: 22
                text: qsTr("")
            }

            Text {
                id: resolution
                x: 32
                y: 187
                width: 48
                height: 22
                text: qsTr("Resolution:")
                font.pixelSize: 12
            }

            Label {
                id: resollabel
                x: 107
                y: 187
                width: 77
                height: 22
                text: qsTr("")
            }

            ComboBox {
                id: colorspace
                x: 30
                y: 84
                model: ["NTSC_1953", "PAL_1970","NTSC_1987","ITU601","ITU601_5",
                        "sRGB","ITU709","ITU709_5","ITU709_7","xvYCC601","xvYCC709",
                        "sYCC601","opYCC601","opRGB","DCI_P3","ITU2020","ITU2020C"]
                width: 140
                height: 35
            }

            ComboBox {
                id: savefile
                x: 26
                y: 352
                model: ["RAW", "RAW+BMP"]
                width: 140
                height: 30
            }

            Text {
                id: name
                x: 32
                y: 307
                width: 48
                height: 22
                text: qsTr("Name:")
                font.pixelSize: 12
            }

            Label {
                id: namelabel
                x: 90
                y: 307
                width: 71
                height: 21
                text: qsTr("")
            }

            RoundButton {
                id: capture
                objectName: "capture-btn"
                x: 51
                y: 406
                width: 72
                height: 48
                text: "Capture"
                signal capture_image(int index, string base, int cnt, bool needbmp)

                function image_meta(dwfourcc, width, height, cs_id) {
                    console.log("capture-done")
                }

                onClicked: {
                    capture_image(videoinputsrc.currentIndex,
                                  pwin.basedir+
                                  Qt.formatDateTime(new Date(), "yyyy-MM-dd-hh:mm:ss.zzz")+
                                  namelabel.text,
                                  1,
                                  savefile.currentIndex)
                }
            }

            RoundButton {
                id: fresh
                x: 26
                y: 234
                width: 72
                height: 48
                text: "Fresh"
            }

            RoundButton {
                id: detail
                objectName: "detail-button"
                x: 107
                y: 234
                width: 72
                height: 48
                text: "Detail"
                function show_detail(str) {
                    mbox.text = str;
                    mbox.show();
                }
            }
        }
    }
}
