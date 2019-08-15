//--------------------------------------------------------------
//! \file My personal implementation of the Model-View-Presenter
//! in c++. MVP is a pattern mainly used by Android.
//--------------------------------------------------------------

#include <iostream>
#include <string>

//--------------------------------------------------------------
//! \brief Aka Model only modifiable by the Presenter. The View
//! is not supposed to know this class.
//--------------------------------------------------------------
class Student
{
public:

  Student() {}
  //! \brief Getter
  std::string const& name() const { return m_name; }
  //! \brief Setter
  void name(std::string const& name) { m_name = name; }

private:

  std::string m_name;
};

//--------------------------------------------------------------
//! \brief In this version no Contract class is made. Presenter
//! replaced the Contract. IView is anyway the Contract.
//--------------------------------------------------------------
class Presenter
{
public:

  //! \brief View contract.
  class IView
  {
  public:

    //! \brief Called when the Presenter wants to add text on the entry widget.
    virtual void setTextOnEntry(std::string const& text) = 0;
    //! \brief Called when the user has entered text on the entry widget
    virtual void onTextEntryChanged(std::string const& text) = 0;
  };

public:

  Presenter(IView& view)
    : m_view(view)
  {
    //! \brief Init the model
    updateName("John Doe");
  }

  //! \brief Update the model and asks the view to refresh
  void updateName(std::string const& name)
  {
    setStudentName(name);
    m_view.setTextOnEntry(getStudentName());
  }

  //! \brief Setter
  void setStudentName(std::string const& name)
  {
    m_student.name(name);
  }

  //! \brief Getter
  std::string const& getStudentName()
  {
    return m_student.name();
  }

  //! \brief Do some computation on the model (here concat strings)
  void compute()
  {
    updateName(getStudentName() + "computed");
  }

private:

  IView&  m_view;
  Student m_student;
};

//--------------------------------------------------------------
//! \brief Concrete implementation of the View.
//--------------------------------------------------------------
class Application: private Presenter::IView
{
public:

  //! \brief Create a fake GUI and runs it
  Application(std::string const& title)
  {
    // Title of the application
    std::cout << "Application title: " << title << std::endl;
    // Because ": m_presenter(*this)" is forbidden
    m_presenter = new Presenter(*this);
    // Simulate a user typing in the entry text widget.
    textEntry();
    // Do computation with the string return by the entry text widget.
    m_presenter->compute();
  }

private:

  //! \brief Concrete implementation
  virtual void setTextOnEntry(std::string const& text) override { std::cout << "Update application widget: " << text << std::endl; }
  //! \brief Concrete implementation
  virtual void onTextEntryChanged(std::string const& text) override { m_presenter->setStudentName(text); };

  //! \brief Simulate a user typing in the entry text widget.
  void textEntry()
  {
    std::cout << "The user is typing some text in a textEntry:" << std::endl;
    std::string txt;
    std::cin >> txt;
    onTextEntryChanged(txt);
  }

private:

  Presenter* m_presenter;
};


int main()
{
  Application app("title");

  return 0;
}
