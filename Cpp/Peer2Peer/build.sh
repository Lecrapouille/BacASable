#! /bin/bash

g++ --std=c++17 -Wall -Wextra -Wshadow *.cpp -o SimCity `pkg-config --cflags --libs sfml-graphics sfml-network`

#./SimCity host 45000
#./SimCity client 45001