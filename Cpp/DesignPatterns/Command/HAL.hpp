#ifndef HAL_HPP
#  define HAL_HPP

#  include "Queue.hpp"
#  include "Medium.hpp"
#  include <thread>
#  include <atomic>
#  include <iostream>

//==============================================================================
//! \brief Define the type for commands the class Medium has to execute.
//==============================================================================
template<typename Return>
using MediumTask = std::function<Return(Medium&)>;

//==============================================================================
//! \brief These lambda functions 'bool f(Medium&)' are used to carry in the
//! queue any general function function to be executed.
//==============================================================================
using MessageQueue = ThreadSafeQueue<MediumTask<bool>>;

//==============================================================================
//! \brief Hardware Layer Abstraction for a generic medium of communication. The
//! medium of communication is supposed to be the single ressource that many
//! clients (ChipController) wants to use for sending and receiving commands on
//! this medium (to communicate with a slave chip ie). To solve this problem
//! this class start its own thread. This thread will get client's commands to
//! execute from a thread safe queue. The executed command allows to send and
//! and receive messages (to the slave chip).
//!
//! \note Since we have decoupled classes, this HAL class does not care about
//! what are concretly are producer and what commands they send, as well as the
//! client receiving the message and answer to it.
//==============================================================================
class HAL
{
public:

    //--------------------------------------------------------------------------
    //! \brief Default constructor. Pure initialization: no action is made here.
    //! The HAL is not functional till the start() method has not been called.
    //--------------------------------------------------------------------------
    HAL(std::shared_ptr<MessageQueue> queue, std::shared_ptr<Medium> medium)
        : m_queue(queue), m_medium(medium)
    {}

    //--------------------------------------------------------------------------
    //! \brief Stop the thread and release memory.
    //--------------------------------------------------------------------------
    ~HAL()
    {
        stop();
    }

    //--------------------------------------------------------------------------
    //! \brief Start the thread. The HAL is now active to treat incoming
    //! commands.
    //--------------------------------------------------------------------------
    void start()
    {
        m_alive = true;
        m_thread = std::thread(&HAL::process, this);
    }

    //--------------------------------------------------------------------------
    //! \brief Stop the thread. The HAL is no longer active and will not treat
    //! incoming commands. Inqueue commands are lost.
    //--------------------------------------------------------------------------
    void stop()
    {
        // Halt the thread loop first
        m_alive = false;

        // Unlock pop()
        m_queue->push([](Medium& /*medium*/) -> bool { return true; });

        // Wait the thread has eneded
        if (m_thread.joinable())
            m_thread.join();
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Threaded process: get each incoming commands from the queue and
    //! execute it.
    //--------------------------------------------------------------------------
    void process()
    {
        while (m_alive)
        {
            if (!exec())
            {
                // Nothing to do
            }
        }
    }

    //--------------------------------------------------------------------------
    //! \brief Get the command from the queue and excute it.
    //--------------------------------------------------------------------------
    bool exec()
    {
        auto fct = m_queue->pop(/*1000_ms*/);
        // Has the queue returned a dummy std::optional value (ie timeout) ?
        if (fct)
        {
            // The task may have throw an exception
            bool res = (*fct)(*m_medium);
            if (!res)
            {
                std::cerr << "HAL: the command has failed to execute"
                          << std::endl;
            }
            return res;
        }
        else
        {
            std::cerr << "HAL: timeout for the command" << std::endl;
            return false;
        }
    }

private:

    std::shared_ptr<MessageQueue> m_queue;
    std::shared_ptr<Medium> m_medium;
    std::atomic<bool> m_alive{false};
    std::thread m_thread;
};

# endif
