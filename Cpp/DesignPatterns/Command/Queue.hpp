#ifndef THREAD_SAFE_QUEUE_HPP
#  define THREAD_SAFE_QUEUE_HPP

#include <optional>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <functional>

using namespace std::literals::chrono_literals;

//==============================================================================
//! \brief Thread safe std::queue (note push and pop name is preferred for stack).
//! \note Ideally this class shall not expose pop() for producer-only like the
//! ChipController class and also shall not expose push() for consumer-only like
//! the HAL class. But here we keep the code simple.
//!
//! \note Beware of destructor: if pop() is blocking you have to push a dummy
//! element to unlock it and therefore unlock the destructor.
//==============================================================================
template<class T>
class ThreadSafeQueue
{
public:

    //--------------------------------------------------------------------------
    //! \brief insert a new element on the queue.
    //--------------------------------------------------------------------------
    void push(T const& element)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(element));
        }

        m_cond.notify_one();
    }

    //--------------------------------------------------------------------------
    //! \brief Get the first element on the queue. This method is blocking for
    //! ever if \c timeout = 0_ms, else a timeout will return an optional value
    //! (this need C++17).
    //--------------------------------------------------------------------------
    std::optional<T> pop(std::chrono::milliseconds timeout = 0ms)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (timeout.count() == 0)
        {
            m_cond.wait(lock, [this]{ return !m_queue.empty(); });
        }
        else
        {
            // wait_for returns false if the return is due to timeout
            auto timeoutOccured = !m_cond.wait_for(lock, timeout,
                [this] {
                    return !m_queue.empty();
                });

            if (timeoutOccured)
            {
                return {};
            }
        }

        T element = m_queue.front();
        m_queue.pop();
        return element;
    }

private:

    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

#if 0
//==============================================================================
//! \brief Class only allowed to produce data in the thread safe queue.
//==============================================================================
template<class T>
class ProducerOnly
{
public:

    ProducerOnly(std::shared_ptr<ThreadSafeQueue<T> q)
        : m_queue(q)
    {}

    inline void push(T const& element)
    {
        m_queue->push(element);
    }

protected:

    std::shared_ptr<ThreadSafeQueue<T> m_queue;
};

//==============================================================================
//! \brief Class only allowed to get data from a thread safe queue.
//==============================================================================
template<class T>
class ConsumerOnly
{
public:

    ConsumerOnly(std::shared_ptr<ThreadSafeQueue<T> q)
        : m_queue(q)
    {}

    inline std::optional<T> pop(std::chrono::milliseconds timeout = 0ms)
    {
        return m_queue->pop(timeout);
    }

protected:

    std::shared_ptr<ThreadSafeQueue<T> m_queue;
};
#endif

#endif
