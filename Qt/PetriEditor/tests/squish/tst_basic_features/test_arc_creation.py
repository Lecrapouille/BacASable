# -*- coding: utf-8 -*-

from squish import *
import test

def test_arc_creation():
    startApplication("PetriNetApp")

    # Create a place and a transition first
    mouseClick(waitForObject(":MainWindow.toolBar.placeButton"))
    mouseClick(waitForObject(":MainWindow.petriNetView"), 100, 100)

    mouseClick(waitForObject(":MainWindow.toolBar.transitionButton"))
    mouseClick(waitForObject(":MainWindow.petriNetView"), 200, 100)

    # Create an arc
    mouseClick(waitForObject(":MainWindow.toolBar.arcButton"))

    # Click on place (source)
    mousePress(waitForObject(":MainWindow.petriNetView"), 100, 100)
    # Drag to transition (target)
    mouseRelease(waitForObject(":MainWindow.petriNetView"), 200, 100)

    # Verify arc was created
    test.verify(findObject(":MainWindow.petriNetView").arcCount == 1,
                "Arc was successfully created")
