## MVC-MVP

Learning how to implement the Model-View-Controller (MVC) and
Model-View-Presenter (MVP) design patterns in C++. These patterns are used for decoupling classes
in Graphical User Interfaces (GUI) like GTK+ or Qt. Model is the class managing your
data. View is the class managing buttons and widgets of the Human-Machine
Interface (HMI). Controller and Presenter are mediator classes between Model and
View.

By "more decoupling" we mean that the library managing the view can be replaced by
another, and therefore Controller/Presenter and Model are View agnostic. More
decoupling means more testable with unit tests.

There are many documents on the internet explaining these two patterns, but I did not
find C++ ultra-basic implementations for MVC and particularly for MVP on GitHub
(MVP is a purely Android pattern), so I'm not 100% sure if the code I made is
correct, so take care! The difficulty with these patterns is to break the
cyclic class relation (A knows B and B knows A). In these examples, I just made
the simplest model possible: a Student class with a single member variable:
its name. The View is emulated by your console, so no additional libraries are
needed for compiling these examples. A simple `g++ -W -Wall -Wextra --std=c++11
*.cpp -o prog` is enough.

Here is the interaction diagram for the MVP:

```
View <--Contract--> Presenter ---> Model
```

Here are explanations:
* A Presenter class is linked to a single View class through a Contract class.

* A Contract describes the communication between view and presenter with
  interface classes (with pure virtual methods). The interface for the Presenter
  is optional.

* The Contract is here for breaking the cyclic class relation by containing
  abstract/interface classes for View and Presenter.

* A Presenter is linked to a single Model.

* The View is passive in the way it only manages widgets, buttons and
  display. It knows nothing about the model.

* A Model is only modified by the Presenter. The Presenter updates the Model
  when the view is updated. The Presenter asks the view to be refreshed when the
  model has been modified by passing to it values of the model.

Advantage/disadvantage:
* Pro: easier to unit test than MVC. The View has no logic other than displaying
  information.

* Drawback: We have to create as many as Presenters than needed Views. Relation
  is 1-to-1.

Here is the interaction diagram for the MVC:

```
View <----> Controler <--> Model
  ^                          |
  |                          |
  +--------------------------+
```

Here are explanations:
* The Controller updates the Model

* The View, through user actions, ask the Controller to update the Model.

* Once changed the Model notifies the Controller and/or the View.

* The Controller can update the View.

* The View can listen Model notifications.

* Notifications are made with a Observer/Observable pattern. A Model is an
  Observable, a Controller is an Observer, the View can be an Observer.

* Thanks to the Observer pattern, a single Controller is enough to serve as
  mediator between multiple Models and multiple Views.

Advantage/disadvantage:
* Drawback: Classes are strongly inter-coupled: this is not pleasant to
  implement it need forward declarations, and pointers: the class View knows
  Controller and View, the class Controller knows a View and a Model.

* More difficult to unit-tests because of the coupling.

Code source:
* 00_MVP: classic version of Model-View-Presenter inspired by Android tutorial
  on MVP. Here Contract class is directly the Presenter class.

* 00_MVP_bis: same than 00_MVP but with a Contract class.

* 01_MVP: modified version of Model-View-Presenter: In this version I did not
  write Contract between the Presenter and the View. No interface classes not
  need of allocation.

* 02_MVP: 2nd modified version of Model-View-Presenter: I added observers. This
  version is not good because it's the Presenter's job to modify the model and
  tell the View to refresh the display of the model. In this example the Model
  only notifies the Presenter, but we can "violate" the design by making the View
  inherit from Observer.

* 03_MVC: Model-View-Controller based on http://www.laputan.org/pub/papers/POSA-MVC.pdf

* 04_MVP-gtk: a GTK+ implementation of the MVP using contracts and multiple
  views on the same Model. I'm not sure if my design is correct where Model is
  an Observable in order to notify all Presenters that its data changed. But
  this is how pygtkmvc is implemented.

* 05_MVP-gtk: Try to add a Proxy class to encapsulate the Student with an
  Observable flavor.

Links:
* http://www.laputan.org/pub/papers/POSA-MVC.pdf
* https://medium.com/@cervonefrancesco/model-view-presenter-android-guidelines-94970b430ddf
* http://pygtkmvc.sourceforge.net/docs/userman/exam.html
