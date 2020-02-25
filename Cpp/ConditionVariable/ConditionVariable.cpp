#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>

// *****************************************************************************
//! \file Simple message passing between two threads. One thread is waiting for
//! a message. The second thread send the message and unlock the first thread.
// *****************************************************************************

// *****************************************************************************
//! \brief Class wrapping std::condition_variable for exchanging a value of type
//! T. The method wait() blocks until notify() unlocks it. notify() sends a
//! value that wait() will return.
// *****************************************************************************
template<class T>
class Notification
{
public:

    //--------------------------------------------------------------------------
    //! \brief Wait for a notification. Is unlocked by notify(T) and return the
    //! value T.
    //--------------------------------------------------------------------------
    T wait()
    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [this] {
            return notified == true;
        });
        notified = false;
        return val;
    }

    //--------------------------------------------------------------------------
    // Send a valued notification. Unlock wait().
    //--------------------------------------------------------------------------
    void notify(T v)
    {
        std::lock_guard<std::mutex> lk(mtx);
        val = v;
        notified = true;
        cv.notify_all();
    }

private:

    //! \brief Value send by notify() to the receiver wait()
    T val{};
    //! \brief Notified.
    bool notified = false;
    mutable std::condition_variable cv;
    mutable std::mutex mtx;
};

// *****************************************************************************
//! \brief Class creating the two threads needed for the example
// *****************************************************************************
class Threadlauncher
{
public:

    //--------------------------------------------------------------------------
    //! \brief Start two threads. The first waits for receiving the
    //! notification, the second send the notification.
    //--------------------------------------------------------------------------
    void run()
    {
        // 1st thread: the notifier
        m_thread = std::thread(&Threadlauncher::notifier, this);

        // main thread: the notified
        int val = m_notif.wait();
        std::cout << "I received " << val << std::endl;

        // Wait for halted threads
        if (m_thread.joinable())
            m_thread.join();
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Send the notification.
    //--------------------------------------------------------------------------
    void notifier()
    {
        std::cout << "Sleep 2s ..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        m_notif.notify(42);
    }

private:

    std::thread m_thread;
    Notification<int> m_notif;
};

//-----------------------------------------------------------------------------
// g++ -std=c++11 -W -Wall -Wextra ConditionVariable.cpp -o cv -pthread
int main()
{
    Threadlauncher a;
    a.run();
    a.run();

    return 0;
}
