//-------------------------------------------------------------------------------
//! \file Observer (signal/slot) using libsigc++
//-------------------------------------------------------------------------------

#include <iostream>
#include <thread>
#include <chrono>

#include <sigc++/sigc++.h>

//-----------------------------------------------------------------------------
// Functions as slots
//-----------------------------------------------------------------------------
static void longOperationFinished()
{
    std::cout << "Worker is finished (function)!" << std::endl;
}

static void longOperationFinishedString(std::string const& message)
{
    std::cout << "Worker is finished (function): '" << message << "'" << std::endl;
}

static int longOperationFinishedInt(int message)
{
    std::cout << "Worker is finished (method) return " << message << std::endl;
    return message + 2;
}

// ****************************************************************************
//! \brief Methods as slots
// ****************************************************************************
class Notifier : public sigc::trackable
{
public:

    void longOperationFinished()
    {
        std::cout << "Worker is finished (method)!" << std::endl;
    }

    void longOperationFinishedString(std::string const& message)
    {
        std::cout << "Worker is finished (method): '" << message << "'" << std::endl;
    }

    int longOperationFinishedInt(int message)
    {
        std::cout << "Worker is finished (method) return " << message << std::endl;
        return message + 1;
    }
};

// ****************************************************************************
//! \brief Class triggering a signal
// ****************************************************************************
class Worker
{
public:

    void doLongOperation()
    {
        // Do a long operation here
        std::cout << "Loooong operation!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Signal the end of the long operation
        m_signal1Finished.emit();
        m_signal2Finished.emit("Hello");
        std::cout << "Res: " << m_signal3Finished.emit(43) << std::endl;
    }

    // Signal 01: no parameter. void is the return type of the method/function
    sigc::signal<void> m_signal1Finished;

    // Signal 02: with string parameter
    sigc::signal<void, std::string> m_signal2Finished;

    // Signal 03: with string parameter and returning an int
    sigc::signal<int, int> m_signal3Finished;
};

// ****************************************************************************
//! \brief Basic example
// ****************************************************************************
void exemple01()
{
    std::cout << "Example 01" << std::endl;
    Worker worker;
    Notifier notifier;

    // Connection to functions
    worker.m_signal1Finished.connect(sigc::ptr_fun(longOperationFinished));
    worker.m_signal2Finished.connect(sigc::ptr_fun(longOperationFinishedString));
    worker.m_signal3Finished.connect(sigc::ptr_fun(longOperationFinishedInt));

    // Connection to methods
    worker.m_signal1Finished.connect(sigc::mem_fun(notifier, &Notifier::longOperationFinished));
    worker.m_signal2Finished.connect(sigc::mem_fun(notifier, &Notifier::longOperationFinishedString));
    worker.m_signal3Finished.connect(sigc::mem_fun(notifier, &Notifier::longOperationFinishedInt));

    worker.doLongOperation();
}

// ****************************************************************************
//! \brief Make the notifier destroyed after its subscription but before
//! receiving its message.
// ****************************************************************************
void exemple02()
{
    std::cout << "Example 02" << std::endl;

    Worker worker;

    // Connection to a function ...
    sigc::connection c = worker.m_signal1Finished.connect(sigc::ptr_fun(longOperationFinished));

    // Destroyed on runtime
    {
        Notifier notifier;
        worker.m_signal1Finished.connect(sigc::mem_fun(notifier, &Notifier::longOperationFinished));
    } // notifier no longer exists

    std::cout << "Reaction 01:" << std::endl;
    worker.doLongOperation(); // function reacts

    std::cout << "Reaction 02:" << std::endl;
    c.disconnect();
    worker.doLongOperation(); // No reaction
}

// ****************************************************************************
//! g++ -W -Wall -Wextra --std=c++11 Sigc.cpp -o Sigc `pkg-config --cflags --libs sigc++-2.0`
// ****************************************************************************
int main()
{
    exemple01();
    exemple02();
    return 0;
}
