#!/bin/bash

mkdir -p build
cd build
qmake6 ../PetriNetViewer.pro
make