#ifndef CHIP_CONTROLLER_HPP
#  define CHIP_CONTROLLER_HPP

#  include "HAL.hpp"
#  include "ChipControllerStateMachine.hpp"
#  include <memory>
#  include <future>
#  include <iostream>

//==============================================================================
//! \brief Class controling a chip by sending to it messages through a medium of
//! communication and getting it answers. Since we suppose there are several
//! chip contollers (one by chip) trying to communicate with their chip we are
//! using a message queue to serialize messages.
//!
//! \note We added a basic state machine (see ChipController.png) to mimic a
//! real situation. The code source for \c ChipControllerStateMachine has been
//! generated from the ChipController.plantuml by an external tool.
//==============================================================================
class ChipController : protected ChipControllerStateMachine
{
public:

    //--------------------------------------------------------------------------
    //! \brief Default constructor. Initialize the context and start the state
    //! machine for communicating with the chip.
    //! FIXME Ideally, the Medium shall not be accessible
    //--------------------------------------------------------------------------
    ChipController(std::shared_ptr<MessageQueue> queue,
                   std::shared_ptr<Medium> medium,
                   std::string const& name)
        : m_queue(queue), m_medium(medium), m_name(name)
    {
        // start the state machine
        ChipControllerStateMachine::enter();
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Send bootloader commands to start the chip. Get answers from it
    //! and depending on the answer change of state.
    //! \note this method is called by the generated state machine.
    //--------------------------------------------------------------------------
    virtual void bootload() override
    {
        // Messages to send to the chip
        std::vector<std::string> const messages = {
            "<start bootloader>",
            "<page1 bootloader>",
            "<page2 bootloader>",
            "<page3 bootloader>",
            "<page4 bootloader>",
            "<page5 bootloader>",
            "<page6 bootloader>",
            "<stop bootloader>",
        };

        // Store answers to show how to deal with delayed answers.
        std::vector<std::future<bool>> answers;
        answers.resize(messages.size());

        // Send all messages in once
        for (size_t i = 0u; i < messages.size(); ++i)
        {
            answers[i] = addTask<bool>([&, i](Medium& medium) -> bool
            {
                std::cout << m_name << " ";

                // Command: Controller --[SPI]--> Chip
                if (!medium.write(messages[i]))
                {
                    std::cerr << m_name << ": Failed to write in the medium"
                              << std::endl;
                    return false;
                }
                // Answer: Chip --[SPI]--> Controller
                bool status;
                std::string answer = medium.read(status);
                if (!status)
                {
                    std::cerr << m_name << ": Failed to write in the medium"
                              << std::endl;
                    return false;
                }
                // Here, we do not care about the answer
                (void) answer;
                return true;
            });
        }

        // Get all answers in once. Be sure that all promise have been read
        // else a segfault will occurs.
        bool success = true;
        for (size_t i = 0u; i < messages.size(); ++i)
        {
            // Negative answer from the chip: abort!
            if (!answers[i].get())
            {
                std::cerr << m_name << ": Failed to bootload because the "
                          << "communication medium failed with the " << i
                          << " message" << std::endl;
                success = false;
            }
        }

        if (success)
        {
            // Positive answer from the chip: we can now interact with the
            // bootloaded application!
            std::cout << m_name << ": Suceeded to bootload" << std::endl;
            ChipControllerStateMachine::triggerSuccess();
        }
        else
        {
            ChipControllerStateMachine::triggerFailure();
        }
    }

    //--------------------------------------------------------------------------
    //! \brief Send application commands to interact with the chip. Get answers
    //! from it and depending on the answer change of state.
    //! \note this method is called by the generated state machine.
    //! FIXME should be an activity (as defined by UML statecharts).
    //--------------------------------------------------------------------------
    virtual void running() override
    {
        std::string const message = "<is alive ?>";

        // In this basic demo, the loop is ended because we w
        while (true)
        {
            std::future<bool> f = addTask<bool>([=](Medium& medium) -> bool
            {
                std::cout << m_name << " ";

                // Command: Controller --[SPI]--> Chip
                if (!medium.write(message))
                {
                    std::cerr << m_name << ": Failed to write in the medium"
                              << std::endl;
                    return false;
                }
                // Answer: Chip --[SPI]--> Controller
                bool status;
                std::string answer = medium.read(status);
                if (!status)
                {
                    std::cerr << m_name << ": Failed to write in the medium"
                              << std::endl;
                    return false;
                }
                // Here, we do not care about the answer
                (void) answer;
                return true;
            });

            // Wait for the answer. If get a negative answer from the chip:
            // abort the loop!
            if (!f.get())
            {
                std::cerr << m_name << ": Failed to run" << std::endl;
                ChipControllerStateMachine::triggerFailure();
                return ;
            }
        }
    }

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    virtual void degraded() override
    {
        std::cerr << m_name << ": degraded mode" << std::endl;
    }

    //--------------------------------------------------------------------------
    //! \brief Push in the queue a function to execute. Once this one will be
    //! executed its result (return code) is stored instead a std::promise.
    //! This allows for the sender to not wait for the result.
    //--------------------------------------------------------------------------
    template<typename Return>
    std::future<Return> addTask(MediumTask<Return> task)
    {
        // This lambda function is used to carry the \c task which is thr real
        // function to execute.
        std::shared_ptr<std::promise<Return>> result =
                std::make_shared<std::promise<Return>>();
        MediumTask<bool> taskWithPromise = [=](Medium& medium)
        {
            try
            {
                // Execute the task and set the result for later usage.
                result->set_value(task(medium));
                return true;
            }
            catch (...)
            {
                std::cerr << m_name
                          << ": Exception occured while executing one task"
                          << std::endl;
                result->set_exception(std::current_exception());
                return false;
            }
        };

        m_queue->push(taskWithPromise);
        return result->get_future();
    }

private:

    std::shared_ptr<MessageQueue> m_queue;
    std::shared_ptr<Medium> m_medium;
    std::string m_name;
};

# endif
