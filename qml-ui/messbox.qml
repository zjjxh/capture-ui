import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

Window {
    id: mbox
    width: 350
    height: 200
    SystemPalette { id: palette }

    TextArea {
        id: textArea
        x: 55
        y: 25
        text:qsTr("")
    }
    title: qsTr("Detail information")
}
