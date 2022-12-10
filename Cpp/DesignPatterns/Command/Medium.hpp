#ifndef MEDIUM_HPP
#  define MEDIUM_HPP

#  include <string>

//==============================================================================
//! \brief Interface class for writing and reading inside a medium (ie. UART,
//! SPI, I2C ...).
//==============================================================================
class Medium
{
public:

    virtual ~Medium() = default;


    //--------------------------------------------------------------------------
    //! \brief Send the given message.
    //! \return true in case of success, else return false.
    //--------------------------------------------------------------------------
    virtual bool write(std::string const& message) = 0;

    //--------------------------------------------------------------------------
    //! \brief Get a message.
    //! \param[inout] status return true in case of success, else return false.
    //! \return return the message. The content shall only interpreted when \c
    //! status has returned true.
    //--------------------------------------------------------------------------
    virtual std::string read(bool& status) = 0;
};

#endif
