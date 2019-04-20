import QtQuick 2.9
import QtQuick.Controls 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4 as Controls14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Styles 1.4 as Styles
import QtQuick.Dialogs 1.1
import org.freedesktop.gstreamer.GLVideoItem 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: qsTr("Capture Demo")
    Messbox{ id:mbox }

    Controls14.SplitView{
        anchors.fill:parent;
        orientation: Qt.Horizontal;
        Rectangle{
            Layout.fillWidth: true;
            color: "black"
            Item {
                anchors.fill: parent
                GstGLVideoItem {
                //Item {
                    id: videoItem
                    objectName: "videoItem"
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height
                }
            }

        }

        Rectangle{
            Layout.minimumWidth: 200
            Layout.maximumWidth: 200
            color: "whitesmoke"
            Controls14.ComboBox {
                id:videoinputsrc
                objectName: "video-input-src"
                anchors.horizontalCenterOffset: 0
                y: 17
                width: 140
                height: 25
                model: ["HDMI-4K-Input", "SDI-4K-Input", "Misc-Input"]
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
                x: 30
                y: 53
                width: 140
                height: 22
                text: qsTr("Pixel Format:")
                font.pixelSize: 12
            }

            TextField {
                id: cclabel
                x: 30
                y: 68
                width: 140
                height: 25
                readOnly: true
                text: qsTr("")
            }

            Text {
                id: resolution
                x: 30
                y: 107
                width: 140
                height: 22
                text: qsTr("Resolution:")
                font.pixelSize: 12
            }

            TextField {
                id: resollabel
                x: 30
                y: 123
                width: 140
                height: 25
                readOnly: true
                text: qsTr("")
            }

            TextField {
                x: 8
                y: 369
                width: 184
                height: 95
                anchors.top: parent.top
                anchors.topMargin: 327
                id: namelabel
                wrapMode: TextArea.WrapAnywhere
                text: "./"
                transformOrigin: Item.Center
            }

            Controls14.Button {
                id: capture
                objectName: "capture-btn"
                x: 120
                y: 434
                width: 72
                height: 29
                text: "Capture"
                property string fname
                signal capture_image(int index, string base, int cnt, bool needbmp, string cslabel)

                function image_meta(dwfourcc, width, height, cs_id) {

                     //console.log("capture-done:"+str)
                     //console.log("fname:"+fname)
                     cs_detect.text = "tmp"
                     cclabel.text = dwfourcc
                     resollabel.text = width + "*" + height
                     if (cslabel.text && cs_detect.text != cslabel.text)
                     {
                        var str = "user input cs:"+cslabel.text+'\n'
                        str += "current cs:"+cs_detect.text+'\n'
                        str += "current width*height:"+width+"*"+height+'\n'
                        str += "current fourcc:"+dwfourcc+'\n'
                        mbox.close()
                        mbox.get_diag_message(str)
                        mbox.show()
                     }

                }

                onClicked: {
                    fname = namelabel.text+
                            Qt.formatDateTime(new Date(), "yyyy-MM-dd-hh:mm:ss:zzz")
                    console.log("fname:"+fname)

                    capture_image(videoinputsrc.currentIndex,
                                  namelabel.text+
                                  Qt.formatDateTime(new Date(), "yyyy-MM-dd-hh:mm:ss:zzz"),
                                  1,
                                  1, cslabel.text)
                }
            }

            Controls14.Button {
                id: fresh
                objectName: "fresh-btn"
                x: 8
                y: 434
                width: 72
                height: 29
                text: "Fresh"
                signal get_fresh()
                onClicked: {
                    console.log("fresh")
                    get_fresh()
                }
                function fresh_meta(dwfourcc, width, height, cs_id) {
                    cs_detect.text = "tmp"
                    cclabel.text = dwfourcc
                    resollabel.text = width + "*" + height
                }
            }

            Text {
                id: inputcs
                x: 30
                y: 162
                width: 140
                height: 22
                text: qsTr("Color Space:")
            }

            TextField {
                id: cslabel
                x: 30
                y: 321
                width: 140
                height: 25
                anchors.top: parent.top
                anchors.topMargin: 236
                text: qsTr("")
            }
            Controls14.Button {
                id: button
                x: 8
                y: 292
                width: 107
                height: 29
                text: qsTr("Open File Path:")
                onClicked: {
                    fds.open();
                }
            }
            FileDialog {
                id:fds
                title: qsTr("Select")
                folder: shortcuts.home
                selectExisting: true
                selectFolder: true
                selectMultiple: false
                onAccepted: {
                    namelabel.text = fds.fileUrl + "/"
                    namelabel.text = namelabel.text.substring(7, fds.fileUrl.length)
                    console.log("You chose: " + fds.fileUrl);
                }

                onRejected: {
                    console.log("Canceled");
                }
            }

            Text {
                id: inputcs1
                x: 30
                y: 219
                width: 140
                height: 22
                text: qsTr("Custom Color Space:")
            }

            TextField {
                id: cs_detect
                x: 30
                y: 328
                width: 140
                height: 25
                text: qsTr("")
                anchors.topMargin: 178
                anchors.top: parent.top
                readOnly: true
            }
        }
    }
}

