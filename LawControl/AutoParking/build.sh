#!/bin/bash

g++ -W -Wall -Wextra --std=c++14 Shapes.cpp Car.cpp main.cpp -o AutoParking `pkg-config --cflags --libs sfml-graphics`
