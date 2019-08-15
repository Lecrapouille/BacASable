#!/bin/bash

g++ -W -Wall -Wextra --std=c++11 00_MVP.cpp -o 00_MVP
g++ -W -Wall -Wextra --std=c++11 00_MVP_bis.cpp -o 00_MVP_bis
g++ -W -Wall -Wextra --std=c++11 01_MVP.cpp -o 01_MVP
g++ -W -Wall -Wextra --std=c++11 02_MVP.cpp -o 02_MVP
g++ -W -Wall -Wextra --std=c++14 03_MVC.cpp -o 03_MVC

# Note: We use c++14 just for std::make_unique, for c++11 add:
# template<typename T, typename... Args>
# std::unique_ptr<T> make_unique(Args&&... args)
# {
#    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
# }
