import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import SpeechApplication 1.0

ApplicationWindow {
    title: qsTr("Speech Apps")
    width: 540
    height: 480
    visible: true

    SpeechController {
        id: speechController
    }

    MainMenu {
        anchors.margins: 5
    }

}
