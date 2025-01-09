import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "white"
    border.color: "lightgray"
    border.width: 1

    property int selectedNodeIndex: -1
    property bool selectedIsPlace: false

    Connections {
        target: petriNetModel
        function onNodeSelected(index, isPlace) {
            selectedNodeIndex = index
            selectedIsPlace = isPlace
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 20

        Label {
            text: {
                if (selectedNodeIndex === -1) {
                    return "Aucun nœud sélectionné"
                } else {
                    return selectedIsPlace ?
                        "Place P" + selectedNodeIndex :
                        "Transition T" + selectedNodeIndex
                }
            }
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
        }
    }
}