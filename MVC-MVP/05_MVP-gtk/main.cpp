#include <string>
#include <iostream>
#include <gtkmm.h>

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
    std::cout << "Attach " << &obs << " to " << this << std::endl;
    m_observers.push_back(&obs);
  }

  void detachObserver(Observer& obs)
  {
    size_t i = 0;
    for (i = 0; i < m_observers.size(); ++i)
      {
        if (m_observers[i] == &obs)
          {
            m_observers.erase(m_observers.begin() + i);
            return ;
          }
      }
  }

  void notifyObservers()
  {
    std::cout << "Notify " << m_observers.size()
              << " observers" << std::endl;
    for (auto& it: m_observers)
    {
      it->update();
    }
  }

private:

  std::vector<Observer*> m_observers;
};

//------------------------------------------------------------------
class Contract
{
public:

  class IView
  {
  public:
    virtual ~IView() {}
    virtual void showName(std::string const& name) = 0;
    virtual Gtk::Button& button() = 0;
  };

  class IPresenter
  {
  public:
    virtual ~IPresenter() {}
    virtual void onClick() = 0;
  };

  class IModel
  {
  public:
    virtual ~IModel() {}
    virtual void nextName() = 0;
    virtual std::string const& name() const = 0;
  };
};

//------------------------------------------------------------------
class Student
  : public Contract::IModel
{
public:

  Student()
  {
    i = m_names.size() - 1;
  }

  virtual std::string const& name() const override
  {
    std::cout << "Student diplay Name " << i
              << ": " << m_names[i] << std::endl;
    return m_names[i];
  }

  virtual void nextName() override
  {
    ++i;
    if (i == m_names.size())
      i = 0;
    std::cout << "Student next index: " << i << std::endl;
  }

private:

  std::vector<std::string> m_names = { "AAA", "BBB", "CCC", "DDD" };
  size_t i;
};

//------------------------------------------------------------------
class ProxyStudent :
  public Student,
  public Observable
{
public:

  ProxyStudent(Student& student)
    : m_student(student)
  {}

  virtual std::string const& name() const override
  {
    return m_student.name();
  }

  virtual void nextName() override
  {
    std::cout << "ProxyStudent Next Name" << std::endl;
    m_student.nextName();
    notifyObservers();
  }

private:

  Student& m_student;
};

//------------------------------------------------------------------
class View :
  public Contract::IView,
  public Gtk::Window
{
public:

  View()
    : m_button("Click me")
  {
    // Window size and position
    set_default_size(200, 200);
    move(px, py);
    px += 300;

    // Pack widgets
    m_vbox.pack_start(m_button);
    m_vbox.pack_start(m_entry);
    add(m_vbox);
    show_all();
  }

  virtual void showName(std::string const& name) override
  {
    m_entry.set_text(name);
  }

  virtual Gtk::Button& button() override
  {
    return m_button;
  }

private:

  Gtk::VBox   m_vbox;
  Gtk::Button m_button;
  Gtk::Entry  m_entry;

  // Window position hack (to avoid to cover themselves)
  static int px;
  static int py;
};

int View::px = 200;
int View::py = 200;

//------------------------------------------------------------------
class Presenter :
  public Contract::IPresenter,
  public Observer
{
public:

  Presenter(ProxyStudent& model, Contract::IView& view) :
    m_model(model),
    m_view(view)
  {
    m_model.attachObserver(*this);
    // TODO: how to hide misery ?
    // m_view.buttonConnect(*this, &Presenter::onClick)
    m_view.button().signal_clicked().connect(sigc::mem_fun(*this, &Presenter::onClick));
  }

private:

  virtual void onClick() override
  {
    m_model.nextName();
  }

  virtual void update() override
  {
    std::cout << "Observer " << this << " Update" << std::endl;
    m_view.showName(m_model.name());
  }

  Contract::IView& view()
  {
    return m_view;
  }

private:

  ProxyStudent& m_model;
  Contract::IView& m_view;
};

//------------------------------------------------------------------
int main()
{
  auto app = Gtk::Application::create();

  Student student;
  ProxyStudent model(student);
  View  v1;
  View  v2;
  Presenter p1(model, v1);
  Presenter p2(model, v2);

  app->signal_startup().connect([&]{
      app->add_window(v2);
    });
  return app->run(v1);
}
