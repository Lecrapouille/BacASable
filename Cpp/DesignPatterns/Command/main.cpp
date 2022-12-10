#include "HAL.hpp"
#include "Medium.hpp"
#include "ChipController.hpp"

//==============================================================================
//! \brief Stubbed medium for testing the global behavior of the Command design
//! pattern without the need to use a real hardware (SPI, I2C ...).
//!
//! For real hardware replace this class by a one wrapping read() / write() /
//! ioctl() functions and include ie "spi_bus.h".
//==============================================================================
class StubMedium : public Medium
{
public:

    StubMedium()
    {
        srand(time(0));
    }

    //--------------------------------------------------------------------------
    //! \brief Simulate sending message on the medium (here never failing).
    //--------------------------------------------------------------------------
    virtual bool write(std::string const& message) override
    {
        std::cout << "StubMedium sent: " << message << std::endl;
        return true;
    }

    //--------------------------------------------------------------------------
    //! \brief Simulate reading a message from the medium. We simulate a failure
    //! for the 40th to 44th message. Previous messages are correctly read.
    //--------------------------------------------------------------------------
    virtual std::string read(bool& status) override
    {
        random_tempo();

        if ((m_count >= 40u) && (m_count <= 44u)) // simulate an error
        {
            std::cout << "StubMedium read FAILURE" << std::endl;
            status = false;
            return {};
        }

        std::string message("answer" + std::to_string(m_count++));
        std::cout << "StubMedium read without failure: " << message << std::endl;
        status = true;
        return message;
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Simulate the delay of answer from the slave chip.
    //--------------------------------------------------------------------------
    void random_tempo()
    {
        int max_number = 100;
        int minimum_number = 1;
        int ms = rand() % (max_number + 1 - minimum_number) + minimum_number;

        std::cout << "Chip is thinking ... " << ms << " ms" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

private:

    //! \brief Message counting
    size_t m_count = 0u;
};

//==============================================================================
//! \brief g++ --std=c++17 -Wall -Wextra -UFSM_DEBUG -DFSM_MOCKABLE *.cpp -o prog -lpthread
//==============================================================================
int main()
{
    // Create an hardware abstraction layer (SPI, I2C ...). This is a single
    // resource that we have to protect against access.
    std::shared_ptr<StubMedium> m = std::make_shared<StubMedium>();
    std::shared_ptr<MessageQueue> q = std::make_shared<MessageQueue>();
    HAL hal(q, m);
    hal.start(); // threaded

    // Create two classes communicating with their slave chip but in concurrence
    // to access to the medium of communication.
    std::thread t1([&](){ ChipController controller2(q, m, "Controller 1"); });
    std::thread t2([&](){ ChipController controller1(q, m, "Controller 2"); });
    std::thread t3([&](){ ChipController controller1(q, m, "Controller 3"); });
    std::thread t4([&](){ ChipController controller1(q, m, "Controller 4"); });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    hal.stop();

    return 0;
}
