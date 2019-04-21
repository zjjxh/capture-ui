import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

/*
MessageDialog {
    id: mbox
    title: "Information"
    text: ""
    function get_diag_message(message) {
        console.log("messagebox-done")
        messageDialog.text = message
    }
    onAccepted: {
        console.log("And of course you could only agree.")
        mbox.quit()
    }
    Component.onCompleted: visible = true
}*/


Window {
    id: mbox
    width: 300
    height: 150
    //SystemPalette { id: palette }

    function get_diag_message(get_message) {
        //console.log("messagebox-done")
        textArea.text = get_message
    }
    TextArea {
        id: textArea
        readOnly: true
        width: 300
        height: 150
        font.pixelSize:20
        text:qsTr("")
    }
    title: qsTr("Information")
}
