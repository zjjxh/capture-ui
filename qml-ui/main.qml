import QtQuick 2.9
import QtQuick.Controls 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4 as Controls14
import QtQuick.Layouts 1.12
import QtQuick.Controls.Styles 1.4 as Styles
//import org.freedesktop.gstreamer.GLVideoItem 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: qsTr("Capture Demo")
    Popup{ id:pwin}
    Messbox{ id:mbox}

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
            //color: "black"
            Item {
                width: 600
                anchors.fill: parent
                /*
                GstGLVideoItem {
                    id: videoItem
                    objectName: "videoItem"
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height
                }*/
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

            TextField {
                id: cclabel
                x: 85
                y: 143
                width: 77
                height: 25
                readOnly: true
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

            TextField {
                id: resollabel
                x: 104
                y: 182
                width: 77
                height: 25
                readOnly: true
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
                y: 414
                model: ["RAW", "RAW+BMP"]
                width: 140
                height: 30
            }

            Text {
                id: name
                x: 32
                y: 369
                width: 48
                height: 22
                text: qsTr("Name:")
                font.pixelSize: 12
            }

            TextField {
                id: namelabel
                x: 76
                y: 369
                width: 77
                height: 25
                anchors.top: parent.top
                anchors.topMargin: 365
                text: qsTr("")
            }

            RoundButton {
                id: capture
                objectName: "capture-btn"
                x: 51
                y: 468
                width: 72
                height: 48
                text: "Capture"
                property string fname
                signal capture_image(int index, string base, int cnt, bool needbmp)

                function image_meta(dwfourcc, width, height, cs_id) {
                    var str = "auto detect fourcc:"+dwfourcc+" width*height:"+width+"*"+height+" colorsapce:"+colorspace.textAt(cs_id)
                     console.log("capture-done:"+str)
                     console.log("fname:"+fname)
                     colorspace.currentIndex = cs_id
                     cclabel.text = dwfourcc
                     resollabel.text = width + "*" + height
                     //if(cslabel.text && colorspace.textAt(cs_id) != cslabel.text)
                     {
                        mbox.get_diag_message(str)
                        mbox.show()
                     }

                }

                onClicked: {
                    fname = pwin.basedir+
                            Qt.formatDateTime(new Date(), "yyyy-MM-dd-hh:mm:ss:zzz")+
                            namelabel.text
                    console.log("fname:"+fname)

                    capture_image(videoinputsrc.currentIndex,
                                  pwin.basedir+
                                  Qt.formatDateTime(new Date(), "yyyy-MM-dd-hh:mm:ss:zzz")+
                                  namelabel.text,
                                  1,
                                  savefile.currentIndex)
                }
            }

            RoundButton {
                id: fresh
                objectName: "fresh-btn"
                x: 26
                y: 234
                width: 72
                height: 48
                text: "Fresh"
                signal get_fresh()
                onClicked: {
                    console.log("fresh")
                    get_fresh()
                }
                function fresh_meta(dwfourcc, width, height, cs_id) {
                    colorspace.currentIndex = cs_id
                    cclabel.text = dwfourcc
                    resollabel.text = width + "*" + height
                }
            }

            RoundButton {
                id: detail
                objectName: "detail-btn"
                x: 107
                y: 234
                width: 72
                height: 48
                text: "Detail"
                signal get_detail()
                onClicked: {
                    console.log("detail")
                    get_detail()
                }
                function detail_meta(str) {
                    mbox.get_diag_message(str)
                    mbox.show()
                }
            }

            Text {
                id: inputcs
                x: 32
                y: 320
                width: 48
                height: 22
                text: qsTr("InputCs:")
                //font.pixelSize:122
            }

            TextField {
                id: cslabel
                x: 88
                y: 321
                width: 77
                height: 25
                anchors.top: parent.top
                anchors.topMargin: 314
                text: qsTr("")
            }
        }
    }
}


