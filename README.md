# BacASable

Share micro projects that I made for learning some techniques that can be used
for biggest projects.

## AutoLoadLib

Interactive Programming in C: edit your C code at run-time, see your changes on
your application without restarting it.  This allows to mimic the C language as
an interpreted language.

## Max-Plus

Defining a Max-Plus class in C++ and Julia for Max-Plus algebra.
More information about this algebra, see https://en.wikipedia.org/wiki/Max-plus_algebra

## Plant Growth

A student project for generating 3D plants for [ScicosLab](http://www.scicoslab.org/)
If you are interested by simulation of plant growth see :
* (fr) http://www.linneenne-lyon.org/depot1/14367.pdf
* (en) https://hal.inria.fr/file/index/docid/71706/filename/RR-4877.pdf

## Design Patterns

My personal implementation of some design patterns because all examples I found
on internet do not respond to concrete cases. For example:
- Model-View-Presenter is pure Android design pattern and I could not find one
  nice and simple implementation in C++, GTKmm.
- Observer nobody in blogs seems to care about the case where the Observer is
  destroyed before the Observable.

### Observer

Safe observer managing the case where the Observer can be destroyed before the
Observable.

### MVC-MVP

Learning how to implement design pattern Model-View-Controller (MVC) and a
Model-View-Presenter (MVP) in C++ first with console then with gtk+.
I tried to write minimalist examples.
