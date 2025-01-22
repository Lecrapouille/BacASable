# PetriNetViewer in Qt6

POC for a Petri Net Viewer in Qt6. Probably will be used in https://github.com/Lecrapouille/TimedPetriNetEditor

![PetriNetViewer](Petri.png)

## Build

```bash
./build.sh
```

## Run

```bash
./build/PetriNetViewer
```

## Prerequisites

```bash
sudo apt install qt6-base-dev qt6-declarative-dev qt6-tools-dev qt6-tools-dev-tools qt6-qmake qml6-module-qtquick-controls qml6-module-qtqml-workerscript qml6-module-qtquick-layouts

sudo apt install \
    qml6-module-qtquick-templates \
    qml6-module-qtquick-controls \
    qml6-module-qtquick \
    qml6-module-qtqml \
    qml6-module-qtquick-layouts \
    qml6-module-qt-labs-platform \
    qml6-module-qtqml-workerscript \
    qml6-module-qtquick-window
```