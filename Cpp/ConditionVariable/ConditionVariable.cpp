#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>

//! \file Send a integer between two threads. One is waiting the other unlock
//! the first.

template<class T>
class Notification
{
public:

  int waitNotification()
  {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [this] {
        return notified == true;
    });
    notified = false;
    return val;
  }

  void notify(int v)
  {
    std::lock_guard<std::mutex> lk(mtx);
    val = v;
    notified = true;
    cv.notify_all();
  }

private:

  T val{};
  bool notified = false;
  mutable std::condition_variable cv;
  mutable std::mutex mtx;
};

class A
{
public:

  A() {}

  void start()
  {
    m_thread = std::thread(&A::runThread, this);

    int val = m_notif.waitNotification();
    std::cout << "I received " << val << std::endl;

    if (m_thread.joinable())
      m_thread.join();
  }

private:

  void runThread()
  {
    std::cout << "Sleep 2s ..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    m_notif.notify(42);
  }

private:

  std::thread m_thread;
  Notification<int> m_notif;
};

// g++ -std=c++11 -W -Wall -Wextra ConditionVariable.cpp -o cv -pthread
int main()
{
  A a;
  a.start();
  return 0;
}
