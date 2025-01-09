import QtQuick 2.15

Item {
    id: root

    property real nodeRadius: 20
    property color placeColor: "white"
    property color transitionColor: "black"
    property color selectedColor: "lightblue"
    property color strokeColor: "black"
    property color arcColor: "black"

    Component.onCompleted: {
        console.log("PetriNetView loaded")
        console.log("petriNetModel available:", petriNetModel !== undefined)
        if (petriNetModel) {
            console.log("Places:", petriNetModel.placePositions.length)
            console.log("Transitions:", petriNetModel.transitionPositions.length)
            canvas.requestPaint()  // Force initial paint
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#e0e0e0"
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        // Add debug logs for size
        Component.onCompleted: {
            requestPaint()
        }

        // Add handler to check if canvas is ready
        onAvailableChanged: {
            if (available) {
                requestPaint()
            }
        }

        Connections {
            target: petriNetModel
            function onPlacePositionsChanged() {
                canvas.requestPaint()
            }
            function onTransitionPositionsChanged() {
                canvas.requestPaint()
            }
            function onArcsChanged() {
                canvas.requestPaint()
            }
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            // Verify that petriNetModel exists
            if (!petriNetModel) {
                console.log("onPaint: petriNetModel is not available!")
                return
            }

            // Draw places
            for (var i = 0; i < petriNetModel.placePositions.length; ++i) {
                var pos = petriNetModel.placePositions[i]
                ctx.beginPath()
                ctx.fillStyle = placeColor
                ctx.strokeStyle = strokeColor
                ctx.lineWidth = 2
                ctx.arc(pos.x, pos.y, nodeRadius, 0, Math.PI * 2)
                ctx.fill()
                ctx.stroke()

                // Draw place name
                ctx.fillStyle = "black"
                ctx.font = "12px sans-serif"
                ctx.textAlign = "center"
                ctx.fillText("P" + i, pos.x, pos.y + nodeRadius + 15)

                // Draw number of tokens
                if (petriNetModel.tokens[i] > 0) {
                    ctx.fillStyle = "black"
                    ctx.font = "14px sans-serif"
                    ctx.textAlign = "center"
                    ctx.fillText(petriNetModel.tokens[i].toString(), pos.x, pos.y + 5)
                }
            }

            // Draw transitions
            for (var i = 0; i < petriNetModel.transitionPositions.length; ++i) {
                var pos = petriNetModel.transitionPositions[i]
                ctx.beginPath()
                ctx.fillStyle = transitionColor
                ctx.strokeStyle = strokeColor
                ctx.lineWidth = 2
                ctx.rect(pos.x - nodeRadius/2, pos.y - nodeRadius/2, nodeRadius, nodeRadius)
                ctx.fill()
                ctx.stroke()

                // Draw transition name
                ctx.fillStyle = "black"
                ctx.font = "12px sans-serif"
                ctx.textAlign = "center"
                ctx.fillText("T" + i, pos.x, pos.y + nodeRadius + 15)
            }

            // Draw arcs
            ctx.strokeStyle = arcColor
            ctx.lineWidth = 2
            for (var i = 0; i < petriNetModel.arcs.length; ++i) {
                var arc = petriNetModel.arcs[i]
                var startPos, endPos
                var startRadius = nodeRadius

                // Calculate start and end positions
                if (arc.sourceIsPlace) {
                    startPos = petriNetModel.placePositions[arc.sourceIndex]
                } else {
                    startPos = petriNetModel.transitionPositions[arc.sourceIndex]
                }

                if (arc.targetIsPlace) {
                    endPos = petriNetModel.placePositions[arc.targetIndex]
                } else {
                    endPos = petriNetModel.transitionPositions[arc.targetIndex]
                }

                // Calculate arc angle
                var dx = endPos.x - startPos.x
                var dy = endPos.y - startPos.y
                var angle = Math.atan2(dy, dx)

                // Adjust start and end points to begin at node borders
                var startX = startPos.x + Math.cos(angle) * nodeRadius
                var startY = startPos.y + Math.sin(angle) * nodeRadius
                var endX = endPos.x - Math.cos(angle) * nodeRadius
                var endY = endPos.y - Math.sin(angle) * nodeRadius

                // Draw the arc
                ctx.beginPath()
                ctx.moveTo(startX, startY)
                ctx.lineTo(endX, endY)
                ctx.stroke()

                // Draw the arrow
                var arrowLength = 10
                var arrowAngle = Math.PI / 6 // 30 degrees

                // Calculate arrow points
                var arrowPoint1X = endX - arrowLength * Math.cos(angle - arrowAngle)
                var arrowPoint1Y = endY - arrowLength * Math.sin(angle - arrowAngle)
                var arrowPoint2X = endX - arrowLength * Math.cos(angle + arrowAngle)
                var arrowPoint2Y = endY - arrowLength * Math.sin(angle + arrowAngle)

                // Draw the arrow
                ctx.beginPath()
                ctx.moveTo(endX, endY)
                ctx.lineTo(arrowPoint1X, arrowPoint1Y)
                ctx.moveTo(endX, endY)
                ctx.lineTo(arrowPoint2X, arrowPoint2Y)
                ctx.stroke()
            }
        }

        MouseArea {
            anchors.fill: parent
            property bool isDragging: false
            property int lastX: 0
            property int lastY: 0

            onPressed: {
                petriNetModel.selectNodeAt(Qt.point(mouseX, mouseY))
                lastX = mouseX
                lastY = mouseY
                isDragging = true
            }

            onReleased: {
                isDragging = false
            }

            onPositionChanged: {
                if (isDragging) {
                    petriNetModel.moveSelectedNode(Qt.point(mouseX, mouseY))
                    lastX = mouseX
                    lastY = mouseY
                }
            }
        }
    }
}