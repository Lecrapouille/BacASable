#include "Publisher.h"
#include <iostream>

void Publisher::publish(int arg1, int arg2) {
    std::cout << "Publisher::publish arg1=" << arg1 << ", arg2=" << arg2 << std::endl;
}