#!/bin/bash

mkdir -p build
cd build
qmake6 ../equipment_manager.pro
make