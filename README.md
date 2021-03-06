# BacASable

Sharing my POC/micro projects that I made for learning some techniques that can be used
for my biggest projects. This is kind of GitHub GISTs but grouped in a single repo (sandbox
means bac à sable in French).

Mainly C++ code but also Julia, Forth, Prolog, Scilab, GTK+ and C code ... some related to C++.

## C code

### AutoLoadLib

Interactive Programming in C: edit your C code at run-time, see your changes on
your application without restarting it. This allows to mimic the C language as
an interpreted language.

## C++

### Forth Wrapper

Prove of concept implementing an ultra-basic Forth interpreter wrapping C++ API.
Two examples are given:
- Wrapping a thin C++ abstraction library integrating the physic engine Bullet and
the 3D library OpenSceneGraph.
- Wrapping a C++ symbolic library Ginac.

### Unit Tests

Getting fun to mock C functions such as open(), read() to unit tests C function calling them.
We exploit the behavior of weak symbols by creating mock functions and define C++ mock class
to use them easily with Google test/mock. See the README in this folder for more details.

### Symbolic Manipulation

Basic symbolic manipulation in C++.

### Runtime Polymorphism

Based on the YT vide "Better Code: Runtime Polymorphism" by Sean Parent.
https://sean-parent.stlab.cc/papers-and-presentations/#better-code-runtime-polymorphism

### ConditionVariable

Test a basic wrapper for notifications through condition variables.

### Union

Test union structure for a Forth project https://github.com/Lecrapouille/SimForth. I dislike the idea of manipulating two separeted stacks: integer and float. Use a single one and make the operator change the type in the same way than OCaml operators `+` and `+.`, `int_of_float`.

### C# Properties

Mimic C# properties in C++.

### OpenCV in C++

Testing some basic functions.

## Prolog

Learning programming Prolog. Learning how to call Prolog from C++ code. Implementing ultra basic Prolog routines based
on graph theory that could be used inside my personal project SimTaDyn instead of heavy C++ class and code.

## Julia

### Max-Plus

Defining a Max-Plus class in C++ and Julia for Max-Plus algebra.
More information about this algebra, see https://en.wikipedia.org/wiki/Max-plus_algebra
My portage to Julia of the Max-Plus Scilab toolbox: https://github.com/Lecrapouille/MaxPlus.jl

## ScicosLab / NSP (Scilab)

http://www.scicoslab.org/

### Plant Growth

A student project for generating 3D plants made long yeeeears ago [NSP](https://cermics.enpc.fr/~jpc/nsp-tiddly/mine.html) or [ScicosLab](http://www.scicoslab.org/). Note: ScicosLab is the fork of Scilab (the equivalent of Matlab) and NSP is now replaced by NSP. This is not a serious application. If you are interested by simulation of plant growth read :
* (fr) http://www.linneenne-lyon.org/depot1/14367.pdf
* (en) https://hal.inria.fr/file/index/docid/71706/filename/RR-4877.pdf

### Scicos blocks

Some "As it" very old and unmaintained C code for ScicosLab: joystick, camera, UART.

## Unity

### Local position vs. world position

Point of view code implementation.

## GTK+

### GTK+2 with GTK-server

Bash script to launch GTK+2 application and GTK-server https://www.gtk-server.org/. Initial step before making the portage for Forth application.

### Design Patterns

My personal implementation of some design patterns because all examples I found
on internet do not respond to concrete cases. For example:
- Model-View-Presenter is pure Android design pattern and I could not find one
  nice and simple implementation in C++, GTKmm.
- Observer nobody in blogs seems to care about the case where the Observer is
  destroyed before the Observable.
- Component pattern for decoupling patterns.
- Visitor pattern.

#### Observer

Safe observer managing the case where the Observer can be destroyed before the
Observable.

Note: Personally I would use [libsigc++](https://developer.gnome.org/libsigc++-tutorial/stable/index.html)
instead of implementing my own Observer/Listener class. This lib allows you to
add signals/slots (similarly to Qt). I give an example of this lib.

#### MVC-MVP

Learning how to implement design pattern Model-View-Controller (MVC) and a
Model-View-Presenter (MVP) in C++ first with console then with gtk+.
I tried to write minimalist examples.

## Gedit: Syntax highlighting

Some *.lang files for my personal projects inspired by https://artisan.karma-lab.net/faire-todo-lists-gedit
- logs: for highlighting log files generated by my personal projects (SimTaDyn ...)
- forth: (TODO) for highlighting my SimForth script (a personal Forth slightly modified)
