# BacASable

Share micro projects that I made for learning some techniques that can be used
for biggest projects (sandbox means bac à sable in French).

## AutoLoadLib

Interactive Programming in C: edit your C code at run-time, see your changes on
your application without restarting it. This allows to mimic the C language as
an interpreted language.

## Max-Plus

Defining a Max-Plus class in C++ and Julia for Max-Plus algebra.
More information about this algebra, see https://en.wikipedia.org/wiki/Max-plus_algebra
My portage to Julia of the Max-Plus Scilab toolbox: https://github.com/Lecrapouille/MaxPlus.jl

## Plant Growth

A student project for generating 3D plants made long yeeeears ago [NSP](https://cermics.enpc.fr/~jpc/nsp-tiddly/mine.html) or [ScicosLab](http://www.scicoslab.org/). Note: ScicosLab is the fork of Scilab (the equivalent of Matlab) and NSP is now replaced by NSP. This is not a serious application. If you are interested by simulation of plant growth read :
* (fr) http://www.linneenne-lyon.org/depot1/14367.pdf
* (en) https://hal.inria.fr/file/index/docid/71706/filename/RR-4877.pdf

## Design Patterns

My personal implementation of some design patterns because all examples I found
on internet do not respond to concrete cases. For example:
- Model-View-Presenter is pure Android design pattern and I could not find one
  nice and simple implementation in C++, GTKmm.
- Observer nobody in blogs seems to care about the case where the Observer is
  destroyed before the Observable.
- Component pattern for decoupling patterns.

### Observer

Safe observer managing the case where the Observer can be destroyed before the
Observable.

Note: Personally I would use [libsigc++](https://developer.gnome.org/libsigc++-tutorial/stable/index.html)
instead of implementing my own Observer/Listener class. This lib allows you to
add signals/slots (similarly to Qt). I give an example of this lib.

### MVC-MVP

Learning how to implement design pattern Model-View-Controller (MVC) and a
Model-View-Presenter (MVP) in C++ first with console then with gtk+.
I tried to write minimalist examples.

## Forth Wrapper

POC Forth wrapping C++ API.

## C++

### ConditionVariable

Test a basic wrapper for notifications through condition variables.

### Union

Test union structure for a Forth project https://github.com/Lecrapouille/SimForth. I dislike the idea of manipulating two separeted stacks: integer and float. Use a single one and make the operator change the type in the same way than OCaml operators `+` and `+.`, `int_of_float`.

## Gedit syntax highlighting

Some *.lang files for my personal projects inspired by https://artisan.karma-lab.net/faire-todo-lists-gedit
- logs: for highlighting log files generated by my personal projects (SimTaDyn ...)
- forth: (TODO) for highlighting my SimForth script (a personal Forth slightly modified)

## GTK+2 with GTK-server

Bash script to launch GTK+2 application and GTK-server https://www.gtk-server.org/. Initial step before making the portage for Forth application.

## Prolog

Learning Prolog. Implement graph theory that could be used inside SimTaDyn.
