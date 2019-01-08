#include <iostream>

//--------------------------------------------------------------
//! \file My personal implementation of the Model-View-Presenter.
//! Contrary to Java, here, the Presenter and View do not interact
//! together throw an common interface class.
//--------------------------------------------------------------

//--------------------------------------------------------------
//! \brief Aka Model
//--------------------------------------------------------------
class Student
{
public:

   Student(std::string const& name) : m_name(name) { }
   // Getter: for the display of the View
   std::string const& name() const { return m_name; }
   // Setter: when the View edits the model
   void name(std::string const& name) { m_name = name; }

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
     std::cout << "View disp Student: " << student.name() << std::endl;
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
class Presenter
{
public:

  // Presenter knows the View and the Model
  Presenter(View& view)
    : m_view(view), m_student("John Doe")
  {
    // Display "John Doe" on the console
    updateView();
  }

  // Setter: update the model and display the
  // new name.
  void setStudentName(std::string const& name)
  {
    m_student.name(name);
    updateView();
  }

  // Getter
  std::string const& getStudentName()
  {
    return m_student.name();
  }

private:

  void updateView()
  {
    m_view.display(m_student);
  }

private:

  View& m_view;
  Student m_student;
};

//--------------------------------------------------------------
//! \brief GUI implements a View and has a Presenter
//--------------------------------------------------------------
class GUI: public View
{
public:

   GUI()
    : m_presenter(*this)
   {
     widget();
   }

private:

   virtual void onTextChanged(std::string const& text) override
   {
      m_presenter.setStudentName(text);
   }

   void widget()
   {
      std::cout << "Student Name:" << std::endl;
   }

private:

  Presenter m_presenter;
};

//--------------------------------------------------------------
//! \brief Simulate the GUI: the user is editing an text entry.
//--------------------------------------------------------------
int main()
{
  GUI gui;
  gui.textEntry();
 
  return 0;
}
