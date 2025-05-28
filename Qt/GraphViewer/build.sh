#!/bin/bash

mkdir -p build
cd build
qmake6 ../GraphViewer.pro
make