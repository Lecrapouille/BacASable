This folder is a proof of concept (POC) on how Forth can wrap a C++ library.

You can find the YouTube video of this code: https://youtu.be/kbVCi5ruCUQ
And the PowerPoint file used for making this video: https://lecrapouille.github.io/forth/ForthWrapper_CppSymbolicEngine.odp

This source code is based on https://github.com/udexon/btosg which is a fork of https://github.com/miguelleitao/btosg in which Forth is used as a wrapper for high-level libraries in different languages (C++, JS, etc.).

There are two versions:
- A C++11 version, but you'll have to create manually all stacks that you need. I'm using dynamic_cast for checking pointer types. dynamic_cast is not liked by developers since it needs RTTI, adding extra information which can slow down performance.
- A C++14 version using template variables, creating automatically all stacks that you need.

To compile my POC code, you will have to follow the following steps:
- git clone https://github.com/miguelleitao/btosg and go to the `cd btosg/examples/` folder.
- Replace the btosg/examples/car.cpp with mine. You can grep for tags `QQ` inside the code to see my additions. Note that I did not care about deleting class instances since this is a POC. I recommend you to use C++ smart pointers instead.
- Adapt the Makefile to replace `c++11` by `c++14`.
- Compile the example (using the btosg makefile): `make`
- Run the application with the following command line: `./carY car: 800 setMass setName MyCar up 3 "*" coord`

Explanation:
- `./carY` is the btosg car application example.
- `car:` creates a new btosg vehicle and pushes it on the stack of btosg objects (I'll name it `OS` for Object Stack).
- `800 setMass` pushes 800 on the Forth data stack (I'll name it `S` for Data Stack as Forth convention) and sets the mass to the vehicle on the top of `OS`.
- `setName MyCar` will give the name `MyCar` to the vehicle on the top of `OS`.
- `up` will push the vector `[0 0 1]` on the `S` stack.
- `3 *` will multiply the Y coordinate. Note I used `"*"` because otherwise bash will interpret it before Forth :(
- `coord` will set the vector as the current position for the vehicle on the top of `OS`.
