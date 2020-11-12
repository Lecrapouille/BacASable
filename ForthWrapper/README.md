This folder is a proof of concept (POC) on how Forth can wrap a C++ library.

This code source is based on https://github.com/udexon/btosg which is a fork of https://github.com/miguelleitao/btosg in which forth is used as wrapper for high level libraries in different langages (C++, JS ..).

To compile my POC code. You will have to follow the following steps:
- git clone https://github.com/miguelleitao/btosg
- replace the btosg/examples/car.cpp by mine. You can grep for tags `QQ` inside the code to see my adds or this [diff](https://github.com/Lecrapouille/BacASable/commit/d0187248f08880649e633fd298447b8bfe68aab4#diff-732ce8b2e88caadf208a672d3d689bea756f24711718fb5da5eb35272bab3175)
- Copy the stack.hpp file on the btosg/examples/ folder.
- Compile the example (using the btosg makefile): `cd btosg/examples; make`
- Run the application with the following command line: `./carY car: 800 setMass setName MyCar up 3 "*" coord`

Explanation:
- `./carY` is the btosg car application example.
- `car:` create a new btosg vehicle and push it on the stack of btosg objects (I'll named it `OS` for Object Stack).
- `800 setMass` push 800 on the Forth data stack (I'll named it `S` for Data Stack as Forth convention) and set to the vehicle on the top of `OS` the mass.
- `setName MyCar` will give the name `MyCar` to the vehicle on the top of `OS`.
- `up` will push the vector `[0 0 1]` on the `S` stack.
- `3 *` will times the Y coordinate. Note I used `"*"` because else the bash will interprete it before Forth :(
- `coord` will set the vector as current position for the vehicle on the top of `OS`.
