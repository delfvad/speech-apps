import QtQuick 2.4

Item {

    function show () {
        console.log("ScreenSettings::show()")
    }

    function free () {
        console.log("ScreenSettings::destroy()")
    }

    Text {
        text: qsTr("Settings")
    }
}

