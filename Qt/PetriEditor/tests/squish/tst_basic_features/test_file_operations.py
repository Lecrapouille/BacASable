# -*- coding: utf-8 -*-

from squish import *
import test

def test_save_and_load():
    startApplication("PetriNetApp")

    # Create a simple Petri net
    mouseClick(waitForObject(":MainWindow.toolBar.placeButton"))
    mouseClick(waitForObject(":MainWindow.petriNetView"), 100, 100)

    # Save the Petri net
    activateItem(waitForObject(":MainWindow.menuBar.fileMenu"), "Save")
    fileDialog = waitForObject(":FileDialog")
    typeText(fileDialog, "test_net.pnml")
    clickButton(waitForObject(":FileDialog.saveButton"))

    # Clear the view
    activateItem(waitForObject(":MainWindow.menuBar.fileMenu"), "New")

    # Load the saved file
    activateItem(waitForObject(":MainWindow.menuBar.fileMenu"), "Open")
    fileDialog = waitForObject(":FileDialog")
    typeText(fileDialog, "test_net.pnml")
    clickButton(waitForObject(":FileDialog.openButton"))

    # Verify the Petri net was loaded correctly
    test.verify(findObject(":MainWindow.petriNetView").placeCount == 1,
                "Petri net was successfully loaded")
