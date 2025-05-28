import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1200
    height: 800
    title: "Graph Viewer with SVG Icons"

    // Graph data - replace with your real data
    property var graphData: {
        "nodes": [
            {"id": "1", "x": 200, "y": 150, "type": "server", "label": "Server 1"},
            {"id": "2", "x": 400, "y": 100, "type": "database", "label": "DB Main"},
            {"id": "3", "x": 600, "y": 200, "type": "client", "label": "Client A"},
            {"id": "4", "x": 300, "y": 300, "type": "router", "label": "Router"},
            {"id": "5", "x": 500, "y": 350, "type": "server", "label": "Server 2"},
            {"id": "6", "x": 700, "y": 300, "type": "client", "label": "Client B"},
            {"id": "7", "x": 450, "y": 450, "type": "database", "label": "DB Backup"}
        ],
        "edges": [
            {"from": "1", "to": "2"},
            {"from": "1", "to": "4"},
            {"from": "2", "to": "3"},
            {"from": "4", "to": "5"},
            {"from": "5", "to": "6"},
            {"from": "5", "to": "7"},
            {"from": "2", "to": "7"}
        ]
    }

    // SVG icons for different node types
    property var svgIcons: {
        "server": "qrc:/icons/server.svg",
        "database": "qrc:/icons/database.svg",
        "client": "qrc:/icons/client.svg",
        "router": "qrc:/icons/router.svg"
    }

    // Component for a single node
    Component {
        id: graphNodeComponent

        Item {
            id: nodeRoot
            width: 50
            height: 50
            z: 10

            property string nodeId
            property string nodeType
            property string nodeLabel
            property string svgSource
            property bool isHovered: false
            property bool isSelected: false

            signal nodeClicked(string nodeId)
            signal nodeDoubleClicked(string nodeId)

            // Shadow
            Rectangle {
                anchors.centerIn: nodeIcon
                width: nodeIcon.width + 4
                height: nodeIcon.height + 4
                radius: width / 2
                color: "#40000000"
                visible: nodeRoot.isHovered || nodeRoot.isSelected
            }

            // SVG icon
            Image {
                id: nodeIcon
                anchors.centerIn: parent
                width: 40
                height: 40
                source: nodeRoot.svgSource
                sourceSize.width: 40
                sourceSize.height: 40
                smooth: true
                antialiasing: true

                // Hover animation
                scale: nodeRoot.isHovered ? 1.1 : 1.0
                Behavior on scale {
                    NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
                }
            }

            // Node label
            Rectangle {
                id: labelBackground
                anchors.top: nodeIcon.bottom
                anchors.topMargin: 5
                anchors.horizontalCenter: parent.horizontalCenter
                width: labelText.contentWidth + 10
                height: labelText.contentHeight + 4
                color: "#80ffffff"
                border.color: "#ccc"
                border.width: 1
                radius: 3
                visible: nodeRoot.isHovered || nodeRoot.isSelected

                Text {
                    id: labelText
                    anchors.centerIn: parent
                    text: nodeRoot.nodeLabel
                    font.pixelSize: 11
                    color: "#333"
                }
            }

            // Click zone
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onEntered: nodeRoot.isHovered = true
                onExited: nodeRoot.isHovered = false

                onClicked: {
                    nodeRoot.nodeClicked(nodeRoot.nodeId)
                    if (mouse.button === Qt.RightButton) {
                        // Context menu if necessary
                        console.log("Right click on:", nodeRoot.nodeId)
                    }
                }

                onDoubleClicked: {
                    nodeRoot.nodeDoubleClicked(nodeRoot.nodeId)
                }
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#f5f5f5"

        // Control zone
        Rectangle {
            id: controlPanel
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 50
            color: "#2c3e50"
            z: 100

            RowLayout {
                anchors.centerIn: parent
                spacing: 20

                Label {
                    text: "Zoom: " + Math.round(graphView.scale * 100) + "%"
                    color: "white"
                }

                Button {
                    text: "Zoom +"
                    onClicked: graphView.zoomIn()
                }

                Button {
                    text: "Zoom -"
                    onClicked: graphView.zoomOut()
                }

                Button {
                    text: "Reset"
                    onClicked: graphView.resetView()
                }

                Label {
                    text: "Nœuds: " + graphData.nodes.length
                    color: "white"
                }
            }
        }

        // Main graph view with zoom and pan
        Flickable {
            id: graphView
            anchors.top: controlPanel.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            contentWidth: graphContainer.width * scale
            contentHeight: graphContainer.height * scale

            property real scale: 1.0
            property real minScale: 0.1
            property real maxScale: 5.0

            // Zoom with the mouse wheel
            MouseArea {
                anchors.fill: parent
                onWheel: {
                    var scaleFactor = wheel.angleDelta.y > 0 ? 1.1 : 0.9
                    var newScale = Math.max(graphView.minScale,
                                  Math.min(graphView.maxScale, graphView.scale * scaleFactor))

                    if (newScale !== graphView.scale) {
                        // Calculate the zoom point
                        var mouseX = wheel.x
                        var mouseY = wheel.y

                        var oldContentX = graphView.contentX
                        var oldContentY = graphView.contentY

                        graphView.scale = newScale

                        // Adjust the position to zoom on the cursor
                        var newContentX = mouseX * (newScale - 1) + oldContentX * scaleFactor
                        var newContentY = mouseY * (newScale - 1) + oldContentY * scaleFactor

                        graphView.contentX = Math.max(0, Math.min(newContentX,
                                           graphView.contentWidth - graphView.width))
                        graphView.contentY = Math.max(0, Math.min(newContentY,
                                           graphView.contentHeight - graphView.height))
                    }
                }
            }

            function zoomIn() {
                scale = Math.min(maxScale, scale * 1.2)
            }

            function zoomOut() {
                scale = Math.max(minScale, scale / 1.2)
            }

            function resetView() {
                scale = 1.0
                contentX = 0
                contentY = 0
            }

            // Main graph container
            Item {
                id: graphContainer
                width: 1000
                height: 600
                scale: graphView.scale
                transformOrigin: Item.TopLeft

                // Background with grid
                Canvas {
                    id: gridCanvas
                    anchors.fill: parent

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.strokeStyle = "#e0e0e0"
                        ctx.lineWidth = 1

                        // Vertical grid
                        for (var x = 0; x <= width; x += 50) {
                            ctx.beginPath()
                            ctx.moveTo(x, 0)
                            ctx.lineTo(x, height)
                            ctx.stroke()
                        }

                            // Horizontal grid
                        for (var y = 0; y <= height; y += 50) {
                            ctx.beginPath()
                            ctx.moveTo(0, y)
                            ctx.lineTo(width, y)
                            ctx.stroke()
                        }
                    }
                }

                // Drawing edges
                Canvas {
                    id: edgesCanvas
                    anchors.fill: parent
                    z: 1

                    property var nodePositions: ({})

                    Component.onCompleted: {
                        // Build a dictionary of node positions
                        for (var i = 0; i < graphData.nodes.length; i++) {
                            var node = graphData.nodes[i]
                            nodePositions[node.id] = {x: node.x, y: node.y}
                        }
                        requestPaint()
                    }

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        ctx.strokeStyle = "#666"
                        ctx.lineWidth = 2

                        // Draw all edges
                        for (var i = 0; i < graphData.edges.length; i++) {
                            var edge = graphData.edges[i]
                            var fromPos = nodePositions[edge.from]
                            var toPos = nodePositions[edge.to]

                            if (fromPos && toPos) {
                                ctx.beginPath()
                                ctx.moveTo(fromPos.x + 25, fromPos.y + 25) // Center of the node
                                ctx.lineTo(toPos.x + 25, toPos.y + 25)
                                ctx.stroke()
                            }
                        }
                    }
                }

                // Graph nodes
                Repeater {
                    model: graphData.nodes
                    delegate: Item {
                        x: modelData.x
                        y: modelData.y

                        Loader {
                            sourceComponent: graphNodeComponent
                            onLoaded: {
                                item.nodeId = modelData.id
                                item.nodeType = modelData.type
                                item.nodeLabel = modelData.label
                                item.svgSource = svgIcons[modelData.type] || svgIcons["server"]

                                item.nodeClicked.connect(function(nodeId) {
                                    console.log("Node clicked:", nodeId, modelData.label)
                                })

                                item.nodeDoubleClicked.connect(function(nodeId) {
                                    console.log("Double-click on node:", nodeId)
                                })
                            }
                        }
                    }
                }
            }
        }
    }
}