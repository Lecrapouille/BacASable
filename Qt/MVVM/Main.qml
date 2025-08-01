import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: "MVVM Image Manager"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Titre
        Text {
            text: "Image Manager (MVVM)"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        // Section d'ajout
        GroupBox {
            title: "Add New Image"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                RowLayout {
                    TextField {
                        id: typeInput
                        placeholderText: "Type"
                        Layout.fillWidth: true
                    }

                    SpinBox {
                        id: identifiantInput
                        from: 0
                        to: 9999
                        value: 1
                    }
                }

                RowLayout {
                    TextField {
                        id: titreInput
                        placeholderText: "Titre"
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Add"
                        enabled: typeInput.text.trim() !== "" && titreInput.text.trim() !== ""
                        onClicked: {
                            imageViewModel.addImageQml(typeInput.text.trim(), identifiantInput.value, titreInput.text.trim())
                            typeInput.clear()
                            identifiantInput.value = 1
                            titreInput.clear()
                        }
                    }
                }
            }
        }

        // Actions globales
        RowLayout {
            Button {
                text: "Clear All"
                onClicked: imageViewModel.clearImagesQml()
            }

            Text {
                text: `Total: ${imageListView.count} images`
                font.pixelSize: 14
                Layout.fillWidth: true
            }
        }

        // Liste des images
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: imageListView
                model: imageViewModel
                spacing: 5

                delegate: Rectangle {
                    width: imageListView.width
                    height: 120
                    color: index % 2 === 0 ? "#f0f0f0" : "#ffffff"
                    border.color: "#cccccc"
                    border.width: 1
                    radius: 5

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5

                        // Informations de l'image
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5

                                RowLayout {
                                    Text {
                                        text: "Type:"
                                        font.bold: true
                                        Layout.minimumWidth: 60
                                    }
                                    TextField {
                                        id: typeField
                                        text: model.type
                                        Layout.fillWidth: true
                                        onEditingFinished: {
                                            if (text !== model.type) {
                                                model.type = text
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Text {
                                        text: "ID:"
                                        font.bold: true
                                        Layout.minimumWidth: 60
                                    }
                                    SpinBox {
                                        id: identifiantField
                                        from: 0
                                        to: 9999
                                        value: model.identifiant
                                        onValueChanged: {
                                            if (value !== model.identifiant) {
                                                model.identifiant = value
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Text {
                                        text: "Titre:"
                                        font.bold: true
                                        Layout.minimumWidth: 60
                                    }
                                    TextField {
                                        id: titreField
                                        text: model.titre
                                        Layout.fillWidth: true
                                        onEditingFinished: {
                                            if (text !== model.titre) {
                                                model.titre = text
                                            }
                                        }
                                    }
                                }
                            }

                            // Bouton de suppression
                            Button {
                                text: "Delete"
                                onClicked: imageViewModel.removeImageQml(index)
                                background: Rectangle {
                                    color: parent.pressed ? "#d32f2f" : "#f44336"
                                    radius: 4
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }
                }
            }
        }

        // Statut
        Text {
            text: "✓ Changes are automatically synchronized between QML and C++"
            color: "#4caf50"
            font.italic: true
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
