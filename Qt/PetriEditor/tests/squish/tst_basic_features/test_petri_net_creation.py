# -*- coding: utf-8 -*-

from squish import *
import test

def main():
    startApplication("PetriNetApp")

    # Test 1: Verify main components are present
    mainWindow = waitForObject(":MainWindow")
    test.verify(mainWindow is not None, "Main window is loaded")

    toolbar = waitForObject(":MainWindow.toolBar")
    test.verify(toolbar is not None, "Toolbar is present")

    petriNetView = waitForObject(":MainWindow.petriNetView")
    test.verify(petriNetView is not None, "PetriNet view is present")

def test_place_creation():
    # Click on Place creation tool
    mouseClick(waitForObject(":MainWindow.toolBar.placeButton"))

    # Create a place by clicking on the view
    mouseClick(waitForObject(":MainWindow.petriNetView"), 100, 100)

    # Verify place was created
    test.verify(findObject(":MainWindow.petriNetView").placeCount == 1,
                "Place was successfully created")

def test_transition_creation():
    # Click on Transition creation tool
    mouseClick(waitForObject(":MainWindow.toolBar.transitionButton"))

    # Create a transition
    mouseClick(waitForObject(":MainWindow.petriNetView"), 200, 100)

    # Verify transition was created
    test.verify(findObject(":MainWindow.petriNetView").transitionCount == 1,
                "Transition was successfully created")
