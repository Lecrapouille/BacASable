#!/bin/bash

g++ -W -Wall -Wextra --std=c++11 main.cpp -o 04_MVP-gtk `pkg-config gtkmm-3.0 --cflags --libs`
