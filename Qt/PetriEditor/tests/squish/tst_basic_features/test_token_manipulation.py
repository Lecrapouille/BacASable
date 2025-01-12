# -*- coding: utf-8 -*-

from squish import *
import test

def test_token_manipulation():
    startApplication("PetriNetApp")

    # Create a place
    mouseClick(waitForObject(":MainWindow.toolBar.placeButton"))
    mouseClick(waitForObject(":MainWindow.petriNetView"), 100, 100)

    # Select the place
    mouseClick(waitForObject(":MainWindow.petriNetView"), 100, 100)

    # Find token spinbox in properties panel
    tokenSpinBox = waitForObject(":MainWindow.propertiesPanel.tokenSpinBox")

    # Set tokens to 3
    tokenSpinBox.setValue(3)

    # Verify token count
    place = findObject(":MainWindow.petriNetView").selectedPlace
    test.verify(place.tokenCount == 3, "Token count was successfully updated")
