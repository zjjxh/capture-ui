import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

Window {
    id: pwin
    width: 350
    height: 200
    SystemPalette { id: palette }
    title: qsTr("Capture directory")

    Button {
           id:openBtn
           x: 14
           y: 47
           height: 25
           text:qsTr("Choose")
           anchors.leftMargin: 10
           onClicked: {
               fds.open();
           }
       }
       TextField {
           id: labels
           y: 48
           width: 239
           height: 24
           text:qsTr("")
           clip: true
           anchors.left:openBtn.right
           anchors.leftMargin: 6
       }

       FileDialog {
           id:fds
           title: "Select"
           folder: shortcuts.home
           selectExisting: true
           selectFolder: true
           selectMultiple: false
           onAccepted: {
               labels.text = fds.fileUrl;
               console.log("You chose: " + fds.fileUrl);
           }

           onRejected: {
               //labels.text = "";
               console.log("Canceled");
           }

       }
}
