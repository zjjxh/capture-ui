import QtQuick 2.9
import QtQuick.Controls 2.1
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
    title: qsTr("Capture")
    Messbox{ id:mbox }

    Controls14.SplitView{
        anchors.fill:parent;
        orientation: Qt.Horizontal;
        Rectangle{
            id: rect1
            Layout.fillWidth: true;
            color: "black"
            Rectangle {
                id:loaderrgb;
                visible: false;
                enabled: visible;
                width: 100;
                height: 30;
                function setCurrentText(textName) {
                    interText.text = textName
                }
                Text {
                    id: interText
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("text")
                }
            }
            Item {
                anchors.fill: parent
                GstGLVideoItem {
                //Item {
                    id: videoItem
                    objectName: "videoItem"
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height
                    signal get_videorgb()
                    function videorgb_meta(rgb)
                    {
                        loaderrgb.setCurrentText(rgb)
                        loaderrgb.x = mouseMA.mouseX;
                        loaderrgb.y = mouseMA.mouseY;
                        loaderrgb.visible = true
                    }
                    MouseArea {
                        id:mouseMA;
                        anchors.fill: parent
                        hoverEnabled: true
                        preventStealing: true
                        onPressed:
                        {
                            videoItem.get_videorgb()
                        }
                        onReleased:
                        {
                            loaderrgb.visible = false
                        }
                    }

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
                y: 19
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

            Controls14.Label{
                id: fourcc
                x: 30
                y: 61
                width: 140
                height: 22
                text: qsTr("Pixel Format:")
            }

            TextField {
                id: cclabel
                x: 30
                y: 76
                width: 140
                height: 25
                readOnly: true
                text: qsTr("")
            }

            Controls14.Label {
                id: resolution
                x: 30
                y: 115
                width: 140
                height: 22
                text: qsTr("Resolution:")
            }

            TextField {
                id: resollabel
                x: 30
                y: 131
                width: 140
                height: 25
                readOnly: true
                text: qsTr("")
            }

            TextField {
                x: 8
                y: 369
                width: 184
                height: 86
                anchors.top: parent.top
                anchors.topMargin: 325
                id: namelabel
                wrapMode: TextArea.WrapAnywhere
                text: "/home/user/Pictures/"
                transformOrigin: Item.Center
            }

            Controls14.Button {
                id: capture
                objectName: "capture-btn"
                x: 120
                y: 417
                width: 72
                height: 29
                text: "Capture"

                property string fname
                signal capture_image(int index, string base, int cnt, bool needbmp, string cslabel)

                function image_meta(dwfourcc, width, height, cs_id) {

                     //console.log("capture-done:"+str)
                     //console.log("fname:"+fname)
                     cs_detect.text = cs_id
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
                    //console.log("fname:"+fname)

                    capture_image(videoinputsrc.currentIndex,
                                  namelabel.text+"-"+
                                  Qt.formatDateTime(new Date(), "yyyy-MM-dd-hh:mm:ss:zzz/"),
                                  8,
                                  1, cslabel.text)
                }
            }

            Controls14.Button {
                id: fresh
                objectName: "fresh-btn"
                x: 8
                y: 417
                width: 72
                height: 29
                text: "Fresh"
                signal get_fresh(int card)
                onClicked: {
                    //console.log("fresh")
                    get_fresh(videoinputsrc.currentIndex)
                }
                function fresh_meta(dwfourcc, width, height, cs_id) {
                    cs_detect.text = cs_id
                    cclabel.text = dwfourcc
                    resollabel.text = width + "*" + height
                }
            }

            Controls14.Label {
                id: inputcs
                x: 30
                y: 170
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
                anchors.topMargin: 243
                text: qsTr("")
            }
            Controls14.Button {
                id: button
                x: 8
                y: 290
                width: 120
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
                    //console.log("You chose: " + fds.fileUrl);
                }

                onRejected: {
                    console.log("Canceled");
                }
            }

            Controls14.Label {
                id: inputcs1
                x: 30
                y: 226
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
                anchors.topMargin: 186
                anchors.top: parent.top
                readOnly: true
            }
        }
    }
}
