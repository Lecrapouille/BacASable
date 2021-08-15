#!/bin/bash -x

rm -fr *.o prog_c prog_cpp

g++ -W -Wall --std=c++11 entry_point.cpp -c
g++ -W -Wall --std=c++11 main_cpp.cpp -c

g++ -W -Wall --std=c++11 -shared -fPIC entry_point.cpp -o libentrypoint.so
g++ -W -Wall --std=c++11 main_cpp.o -o prog_cpp -L. -lentrypoint

gcc -W -Wall main_c.c -c
gcc -W -Wall entry_point.o main_c.o -o prog_c -lstdc++

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
./prog_cpp
