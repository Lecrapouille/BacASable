import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "#f0f0f0"

    Component.onCompleted: {
        console.log("Main QML loaded - Size:", width, "x", height)
        console.log("PetriNetView size:", petriView.width, "x", petriView.height)
        if (petriNetModel) {
            console.log("PetriNetModel is available")
            petriNetModel.requestUpdate()
        } else {
            console.log("petriNetModel not available!")
        }
    }

    PetriNetView {
        id: petriView
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: infoPanel.top
        }
    }

    NodeInfoPanel {
        id: infoPanel
        height: 100
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }
}