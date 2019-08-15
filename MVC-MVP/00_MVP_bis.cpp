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
//! \brief In this version a Contract class is used.
//! IView has always to do the Contract.
//! IPresenter is optional.
//--------------------------------------------------------------
class Contract
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

  //! \brief Presenter contract.
  class IPresenter
  {
  public:

    virtual void updateName(std::string const& name) = 0;
  };
};

//--------------------------------------------------------------
//! \brief Concrete Presenter
//--------------------------------------------------------------
class Presenter: public Contract::IPresenter
{
public:

  Presenter(Contract::IView& view)
    : m_view(view)
  {
    //! \brief Init the model
    updateName("John Doe");
  }

  virtual void updateName(std::string const& name) override
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

  Contract::IView& m_view;
  Student          m_student;
};

//--------------------------------------------------------------
//! \brief Concrete implementation of the View.
//--------------------------------------------------------------
class Application: private Contract::IView
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
