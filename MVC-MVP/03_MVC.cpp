#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cassert>

//--------------------------------------------------------------
//! \brief My personal implementation of the Model-View-Controller
//! based on http://www.laputan.org/pub/papers/POSA-MVC.pdf
//--------------------------------------------------------------

//--------------------------------------------------------------
class Observer
{
public:
  virtual ~Observer() {}
  virtual void update() = 0;
};

//--------------------------------------------------------------
class Observable
{
public:

  void attachObserver(Observer& obs)
  {
    m_observers.push_back(&obs);
  }

  void detachObserver(Observer& obs)
  {
    size_t i = 0;
    for (i = 0; i < m_observers.size(); ++i)
      {
        if (m_observers[i] == &obs)
          {
            std::cout << "detach" << std::endl;
            m_observers.erase(m_observers.begin() + i);
            return ;
          }
      }
  }

  void notifyObservers()
  {
    for (auto& it: m_observers)
    {
      it->update();
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

  // The Model initialize data
  Student(std::string const& name)
  {
    // Note: this notification cannot be caught because observers
    // are not yet attached.
    m_name = name;
  }

  // Getter
  std::string const& name() const { return m_name; }

  // Setter + notify observers
  void name(std::string const& name)
  {
    m_name = name;
    notifyObservers();
  }

private:

   std::string m_name;
};

//--------------------------------------------------------------
// Forward declaration
class Controller;
class Event {};

//--------------------------------------------------------------
// Abstract class
class View : public Observer
{
public:

  // Unfortunatly we cannot init m_controller with makeController()
  // because it's dangerous to call virtual methods from the
  // constructor.
  View(Student& student)
    : m_student(student),
      m_controller(nullptr)
  {
    m_student.attachObserver(*this);
  }

  ~View()
  {
    m_student.detachObserver(*this);
  }

  inline Student& getStudent()
  {
    return m_student;
  }

  inline Controller& getController()
  {
    assert(nullptr != m_controller);
    return *(m_controller.get());
  }

  void setController(std::unique_ptr<Controller> controller)
  {
    m_controller = std::move(controller);
  }

  // To be called after constructor
  virtual void init()
  {
    m_controller = std::move(makeController());
  }

  virtual std::unique_ptr<Controller> makeController()
  {
    return std::make_unique<Controller>(*this);
  }

  virtual void update() override
  {
    display();
  }

private:

  // Refresh the view
  virtual void display() = 0;

  Student& m_student;
  std::unique_ptr<Controller> m_controller;
};

//--------------------------------------------------------------
// Abstract class
class Controller: public Observer
{
public:

  Controller(View& view)
    : m_view(view),
      m_student(view.getStudent())
  {
    m_student.attachObserver(*this);
  }

  ~Controller()
  {
    m_student.detachObserver(*this);
  }

  virtual void update() override
  {
    // Do nothing by default
  }

  // Simulate an event
  virtual void handleEvent(Event& /*e*/) {};

protected:

  View& m_view;
  Student& m_student;
};

//--------------------------------------------------------------
// Implement the first concrete Controller
class ImpCtrl1: public Controller
{
public:

  ImpCtrl1(View& view)
    : Controller(view)
  {
  }

  virtual void handleEvent(Event& /*e*/) override
  {
    // if (e == ) { ...
    std::cout << "ImpCtrl1: got an event" << std::endl;
    // Change the name
    onRequest("New Name " + std::to_string(i++));
  }

  virtual void update() override
  {
    std::cout << "ImpCtrl1: update because model changed" << std::endl;
  }

private:

  void onRequest(std::string const& studentName)
  {
    m_student.name(studentName);
  }

  int i = 0;
};

//--------------------------------------------------------------
// Implement the second concrete Controller
class ImpCtrl2: public Controller
{
public:

  ImpCtrl2(View& view)
    : Controller(view)
  {
  }

  virtual void handleEvent(Event& /*e*/) override
  {
    // if (e == ) { ...
    std::cout << "ImpCtrl2: got an event" << std::endl;
    // Change the name
    if (i == 0)
      onRequest("Fanfan LaTulipe");
    else
      onRequest("Paul Jack");
    i = (i + 1) % 2;
  }

  // This controller do not react
  // virtual void update() override

private:

  void onRequest(std::string const& studentName)
  {
    m_student.name(studentName);
  }

  int i = 0;
};

//--------------------------------------------------------------
// Implement the first concrete View
class ImpView1: public View
{
public:

  ImpView1(Student& student)
    : View(student)
  {}

  virtual std::unique_ptr<Controller> makeController() override
  {
    return std::make_unique<Controller>(*this);
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

  virtual void display() override
  {
    std::cout << "View1 disp Student " << getStudent().name() << std::endl;
  }

  // Simulate the key pressed event on an entry text
  void onTextChanged(std::string const& text)
  {
    getStudent().name(text);
  }
};

//--------------------------------------------------------------
class GUI
{
public:

  GUI()
    : m_student("John Doe"),
      m_view(m_student)
  {
    m_view.setController(std::make_unique<ImpCtrl2>(m_view));
    m_student.notifyObservers();
  }

  inline Controller& getController()
  {
    return m_view.getController();
  }

  inline Student& getStudent()
  {
    return m_student;
  }

  inline View& getView()
  {
    return m_view;
  }

  Student   m_student;
  ImpView1  m_view;
};

//--------------------------------------------------------------
int main()
{
  GUI gui;

  std::cout << "--- Simulate model change ---" << std::endl;
  gui.getStudent().name("Pierre Dupont");

  std::cout << "--- Simulate GUI event ---" << std::endl;
  Event e;
  gui.getController().handleEvent(e);
  gui.getController().handleEvent(e);
  gui.getController().handleEvent(e);

  std::cout << "--- Replace the controller ---" << std::endl;
  gui.getView().setController(std::make_unique<ImpCtrl1>(gui.m_view));

  gui.getController().handleEvent(e);
  gui.getController().handleEvent(e);
  gui.getController().handleEvent(e);

  return 0;
}
