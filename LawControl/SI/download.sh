#!/bin/bash -e

#### Library for unit conversion library
#### License: MIT
git clone https://github.com/nholthaus/units.git --depth=1
(cd units/include && mkdir units && cp units.h units/units.hpp)

#### A header only C++ library that provides type safety and user defined literals for physical units 
#### License: MIT
git clone https://github.com/bernedom/SI.git --depth=1
