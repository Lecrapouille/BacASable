# BacASable

Share micro projects that I made for learning some techniques.

## AutoLoadLib

Interactive Programming in C: edit your C code at runtime, see your changes on your application without restarting it.
This allows to mimic the C langage as an interprete langage.

## Max-Plus

Defining a MaxPlus class in C++ and Julia:
https://en.wikipedia.org/wiki/Max-plus_algebra

## MVC-MVP

Learning how to implement a Model-View-Controller (MVC) and a Model-View-Presenter (MVP) in C++. I'm not sure if the design I made is the correct one, so take care (anyway I found no C++ solution for MVP on github, google)! In these examples, I just made the simplest model possible: a Student class with its a single member variable : its name.

Here is the interaction diagram for the MVP:

```
View <--Contract--> Presenter ---> Model
```

Here are explanations:
* A Presenter is linked to a single View by a Contract. 
* A Presenter is linked to one or several Models.
* A Model is only modified by a Presenter. The Presenter asks the View to refresh the display of the Model.
* A Contract is 
* Pro: easy to implement and to unit test. Use mock of the Contract for unit tests. The View has no logic other than displaying informations and sending signals to the Presenter. The Presenter connects signals of the view to its slots.
* Drawback: We have to create as many as Presenters than Views.

Here is the interaction diagram for the MVC:

```
View <----> Controler <--> Model
  ^                          |
  |                          |
  +--------------------------+
```

Here are explanations:
* The Controler updates the Model
* The view, through user actions, ask the Controller to update the Model.
* Once changed the Model notifies the Controller and/or the View.
* The Controler can update the View.
* The View can listen Model notifications.
* Notifications are made with a Observer/Observable pattern. A Model is an Observable, a Controler is an Observer, the View can be an Observer.
* Thanks to the Observer pattern, a single Controller is enough to serve as mediator between multiple Models and multiple Views.
* Drawback: Classes are strongly inter-coupled: this is not pleasant to implement it need forward declarations, and pointers: the class View knows Controller and View, the class Controller knows a View and a Model. 

Code source:
* 01_MVP: 1st version of Model-View-Presenter: In this version I did not write Contract between the Prensenter and the View.
* 02_MVP: 2nd version of Model-View-Presenter: with observers. This version is not good because it's the Presenter job to modify the model and tells the View to refresh the display of the model. In this example the Model only notifies the Presenter but we can "violate" the design by making the View to inherit from 
* 03_MVC: Model-View-Controller based on http://www.laputan.org/pub/papers/POSA-MVC.pdf
* 04_MVP-gtk: a GTK+ implementation of the MVP using contracts and multiple views on the same Model. I'm not sure if my design is correct where Model is an Observable in the aim to prevent all Presents that it's data changed. But this how pygtkmvc is implemented.
* 05_MVP-gtk: Try to add a Proxy class to encapsulate the Student with an Observable flavor.

Links:
* http://www.laputan.org/pub/papers/POSA-MVC.pdf
* https://medium.com/@cervonefrancesco/model-view-presenter-android-guidelines-94970b430ddf
* http://pygtkmvc.sourceforge.net/docs/userman/exam.html
