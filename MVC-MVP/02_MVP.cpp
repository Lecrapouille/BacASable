#include <iostream>
#include <vector>

//--------------------------------------------------------------
//! \brief My personal implementation of the Model-View-Presenter
//! with observers class: the model notifies when its data changed.
//! (x) where x is a number is the order of interactions between
//! the Model, View, Presenter.
//--------------------------------------------------------------

//--------------------------------------------------------------
class Observer
{
public:
  virtual void update(std::string const& str) = 0;
};

//--------------------------------------------------------------
class Observable
{
public:

  void addObserver(Observer& obs)
  {
    m_observers.push_back(&obs);
  }
  void notifyObserver(std::string const& str)
  {
    for (auto& it: m_observers)
    {
      it->update(str);
    }
  }

private:

  std::vector<Observer*> m_observers;
};

//--------------------------------------------------------------
// Aka Model
class Student: public Observable
{
public:

   Student(std::string const& name) : m_name(name) { }
   // Constructor: add a single Observer (single: to stay simple)
   // Possibly: Presenter, Controller and View can be an Observer.
   Student(Observer& obs, std::string const& n)  { addObserver(obs); name(n); }
   // Getter: for the display of the View
   std::string const& name() const { return m_name; }
   // Setter + notify observers: when the View edits the model
   void name(std::string const& name) { m_name = name; notifyObserver(m_name); }

private:

   std::string m_name;
};

//--------------------------------------------------------------
//! \brief View. We simulate a GUI with the console:
//! - When the user edits the model we simulate by reading the cin
//! - When the view display the model we write in cout.
//--------------------------------------------------------------
class View
{
public:

   // Simulate the text refresh of an entry text widget by writing chars
   // in the console.
   void display(Student const& student) const
   {
     std::cout << "View disp Student " << student.name() << std::endl;
   }

   // Simulate the user typing in an entry text widget by reading chars in
   // the console.
   void textEntry()
   {
      std::cout << "The user is typing some text in a textEntry" << std::endl;
      std::string txt;
      std::cin >> txt;
      onTextChanged(txt);
   }

private:

   // Simulate the callback 'on key pressed event' of an entry text
   virtual void onTextChanged(std::string const& text) = 0;
};

//--------------------------------------------------------------
//! \brief the Presenter is the mediator between the model and the
//! view. It responds to the view callbacks and update the model.
//--------------------------------------------------------------
class Presenter: private Observer 
{
public:

  // Presenter knows the View and the Model
  Presenter(View& view)
    : m_view(view), m_student(*this, "John Doe")
  {
  }

  // Setter: update the model.
  void setStudentName(std::string const& name)
  {
    m_student.name(name);
  }

  // Getter
  std::string const& getStudentName()
  {
    return m_student.name();
  }

  // Getter for the GUI
  Student& getStudent()
  {
    return m_student;
  }

private:

  void updateView()
  {
    m_view.display(m_student);
  }

  virtual void update(std::string const& studientName) override
  {
    m_view.display(studientName);
  }

private:

  View& m_view;
  Student m_student;
};

//--------------------------------------------------------------
class GUI: public View
{
public:

   GUI()
    : m_presenter(*this)
   {
   }

  Student& getStudent()
  {
    return m_presenter.getStudent();
  }

private:

   virtual void onTextChanged(std::string const& text) override
   {
      m_presenter.setStudentName(text);
   }

private:

  Presenter m_presenter;
};

//--------------------------------------------------------------
int main()
{
  GUI gui;

  // Simulate the model is updating itself: the view is notified
  // and display the new name.
  std::cout << "--- Simulate model change ---" << std::endl;
  Student s = gui.getStudent();
  s.name("Pierre Dupont");

  // Simulate the GUI: the user is typing in an entry text widget.
  // The Model is updated and the new name is displayed.
  std::cout << "--- Simulate GUI event ---" << std::endl;
  gui.textEntry();
 
  return 0;
}
