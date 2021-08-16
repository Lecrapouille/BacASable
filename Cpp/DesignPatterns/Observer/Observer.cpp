//-------------------------------------------------------------------------------
//! \file My personal C++ implementation of the Observer pattern managing the
//! case where observers can be destroyed before the observable. The huge
//! problem to deal with is that Observer and Observable cross reference
//! themself.
//------------------------------------------------------------------------------

#include <set>
#include <iostream>
#include <string>
#include <mutex>
#include <cassert>

// Forward declaration
class IObservable;
class ISafeObserver;

// *****************************************************************************
//! \brief Classic Abstract Observer.
//!
//! An Observer react to changes of the Observable class through the notification
//! method update().
//!
//! In this example I added a name to the Observer for tracing their actions.
// *****************************************************************************
class IObserver
{
public:

    //! \brief Constructor in this example Observers have a name.
    IObserver(std::string const& name)
      : m_name(name)
    {
       std::cout << "Observer '" << m_name << "' created" << std::endl;
    }

    //! \brief Virtual destruction because of pure virtual method update()
    virtual ~IObserver()
    {
      std::cout << "Observer '" << m_name << "' destroyed" << std::endl;
    }

    //! \brief Getter
    std::string const& name()
    {
       return m_name;
    }

    //! \brief method triggered when the Observable has changed of state
    virtual void update() = 0;

private:

    //! \brief just use for the debug and traces
    std::string m_name;
};

// *****************************************************************************
//! \brief Classic Abstract Observable.
//!
//! An Observable manages a list of Observers and to notify them of state
//! changes by calling their update() method.
//!
//! Note that this class manages ISafeObserver (decalred fast forward) but not
//! directly the previously defined class IObserver!
// *****************************************************************************
class IObservable
{
public:

    virtual ~IObservable() {}
    virtual void attachObserver(ISafeObserver* obs) = 0;
    virtual void detachObserver(ISafeObserver* obs) = 0;
    virtual void detachRandomObserver() = 0;
    virtual void detachAllObservers() = 0;
    virtual void notifyAllObservers() = 0;
};

// *****************************************************************************
//! \brief Abstract class for the safe version of Observer.
//!
//! This Observer knows the Observable to prevent it when it's going to be
//! destroyed (destructor).
// *****************************************************************************
class ISafeObserver: public IObserver
{
private:

    // Be careful be friend with the concrete class (not yet defined) and not
    // the previously defined IObservable class!
    friend class Observable;

    // Hold the which Observable this observer is attached to. A possible evolution
    // is to use a container (std::set or std::list, std:: vector ...) in the case
    // the observer is hold by several observables.
    IObservable* m_observable = nullptr;

public:

    ISafeObserver(std::string const& name)
      : IObserver(name)
    {}

    //! \brief When the instance is destroyed prevent the owner to remove it from
    //! the list of observers.
    ~ISafeObserver()
    {
       //std::cout << "ISafeObserver '" << name() << "' is destroyed" << std::endl;
       if (m_observable != nullptr)
         m_observable->detachObserver(this);
       assert(m_observable == nullptr); // made by detachObserver()
    }

    //! \brief this method is just for checking if this instance is holded by an
    //! Observable.
    void isOwned()
    {
       std::cout << (m_observable == nullptr ? "NO" : "YES") << std::endl;
    }
};

// *****************************************************************************
//! \brief Concrete Observable. Implement how Observers are stored in container
// *****************************************************************************
class Observable: public IObservable
{
public:

    ~Observable()
    {
       std::cout << "Observable is gonna be destroyed {" << std::endl;
       detachAllObservers();
       std::cout << "} Observable is gonna be destroyed" << std::endl;
    }

    // -------------------------------------------------------------------------
    virtual void attachObserver(ISafeObserver* obs) override
    {
        std::lock_guard<std::mutex> guard{m_mutex};
        std::cout << "Attaching Observer '" << obs->name() << "'" << std::endl;

        // A std::set manages natively duplicated elements.
        m_observers.insert(obs);
        obs->m_observable = this; // owned

        std::cout << "   Count: " << m_observers.size() << " observers" << std::endl;
    }

    // -------------------------------------------------------------------------
    virtual void detachObserver(ISafeObserver* obs) override
    {
        std::lock_guard<std::mutex> guard{m_mutex};
        std::cout << "Detaching Observer '" << obs->name() << "'" << std::endl;

        m_observers.erase(obs);
        obs->m_observable = nullptr; // no owner

        std::cout << "   Count: " << m_observers.size() << " observers" << std::endl;
    }

    // -------------------------------------------------------------------------
    // This method is for Example 02
    virtual void detachRandomObserver() override
    {
        std::lock_guard<std::mutex> guard{m_mutex};

        if (m_observers.size() == 0)
        {
          std::cout << "Detaching empty set" << std::endl;
          return ;
        }
        auto it = m_observers.begin();
        std::advance(it, rand() % m_observers.size());
        std::cout << "Detaching Observer '" << (*it)->name() << "'" << std::endl;

        (*it)->m_observable = nullptr;
        m_observers.erase(it);

        std::cout << "   Count: " << m_observers.size() << " observers" << std::endl;
    }

    // -------------------------------------------------------------------------
    virtual void detachAllObservers() override
    {
        std::lock_guard<std::mutex> guard{m_mutex};
        std::cout << "Detaching all observers" << std::endl;

        for (auto const& obs: m_observers)
            obs->m_observable = nullptr; // no owner
        m_observers.clear();

        std::cout << "   Count: " << m_observers.size()  << " observers" << std::endl;
    }

    // -------------------------------------------------------------------------
    virtual void notifyAllObservers() override
    {
        std::lock_guard<std::mutex> guard{m_mutex};
        std::cout << "Notify all observers:" << std::endl;
        for (auto const& obs: m_observers)
            obs->update();
    }

protected:

    //! \brief A std::set manages natively duplicated elements.
    std::set<ISafeObserver*> m_observers;

    //! \brief Thread safe
    std::mutex m_mutex;
};

// *****************************************************************************
//! \brief Concrete implementation of the Observer.
//! Implement the update() method.
// *****************************************************************************
class SafeObserver: public ISafeObserver
{
public:

    SafeObserver(std::string const& name)
      : ISafeObserver(name)
    {}

    virtual void update() override
    {
       std::cout << "  Observer '" << name() << "' has been notified" << std::endl;
    }
};

// *****************************************************************************
//! \brief EXAMPLE 01 Create/delete observers. See if Observable removed destroyed
//! observers.
// *****************************************************************************

//! \brief Define your application here. In this example make it dummy.
class Example01: public Observable {};

//! \brief Create/delete observers
static void example01()
{
   std::cout << "Example 01" << std::endl;
   SafeObserver* obs0 = new SafeObserver("obs0");
   obs0->isOwned(); // "No" because not owned by an observable

       Example01* app = new Example01();
       app->attachObserver(obs0); // ==> [ obs0 ]
       obs0->isOwned(); // "Yes": because owned by an observable

           SafeObserver* obs1 = new SafeObserver("obs1");
           app->attachObserver(obs1); // ==> [ obs0, obs1 ]

               SafeObserver* obs2 = new SafeObserver("obs2");
               app->attachObserver(obs2); // ==> [ obs0, obs1, obs2 ]
               app->notifyAllObservers();

               delete obs2; // ==> [ obs0 obs1 ]
               app->notifyAllObservers();

           delete obs1; // ==> [ obs0 ]
           app->notifyAllObservers();
       delete app; // Application destroyed before obs0
       obs0->isOwned(); // "No": because obs0 is no longer owned by app
   delete obs0;
}

// *****************************************************************************
//! \brief EXAMPLE 02
// *****************************************************************************

#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <stdio.h>
#include <stdlib.h>

class Example02: public Observable {};
static std::atomic<size_t> counter{0};
static Example02 app;
static std::atomic<bool> end{false};
static std::unique_ptr<SafeObserver> list[64];

static void call_from_thread(uint32_t const i)
{
  while (!end)
  {
      if (i == 0)
        {
          // Create new observer and attach it. On colision, the new observer
          // smashes the oder which is destroyed by the smart pointer and then
          // detached.
          size_t c = counter++;
          size_t i = c % ((rand() % 63) + 1);
          list[i].reset(new SafeObserver("obs" + std::to_string(c)));
          app.attachObserver(list[i].get());
        }
      else if (i == 1)
        app.detachRandomObserver();
      else if (i == 2)
        app.notifyAllObservers();
      std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
  }
}

static void example02()
{
   std::cout << "Example 02" << std::endl;

   constexpr uint32_t num_threads = 3U;
   std::thread t[num_threads];

   // Launch a group of threads
   for (uint32_t i = 0; i < num_threads; ++i)
   {
       t[i] = std::thread(call_from_thread, i);
   }

   std::this_thread::sleep_for(std::chrono::seconds(5));
   end = true;

   // Join the threads with the main thread
   for (uint32_t i = 0; i < num_threads; ++i)
   {
        if (t[i].joinable())
        {
            t[i].join();
        }
   }
}

// g++ --std=c++11 -W -Wall -Wextra Observer.cpp -o prog -pthread
int main()
{
   srand(time(nullptr));
   example01();
   example02();
   return 0;
}
