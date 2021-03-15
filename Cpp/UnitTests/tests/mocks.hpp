#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

//******************************************************************************
//! \brief Class allowing either to mock the C function read() or to call the
//! real function. This class implements a pseudo singleton: if the singleton
//! instance is nullptr then the real read function is called, else the mock
//! method is called.
//!
//! \note Up to you to implement a more explicit way to select between mock and
//! real symbol.
//******************************************************************************
class ReadMock
{
public:

    //--------------------------------------------------------------------------
    //! \brief when calling explicitly the constructor this will force invoking
    //! the mock. If the real function is prefered then do not called this
    //! constructor: the instance is set to nullptr and this will force loading
    //! the real symbol from dlopen.
    //! \note Contrary to a real singleton the constrcuctor is public: this is
    //! made for selecting between mock and real function.
    //--------------------------------------------------------------------------
    ReadMock()
    {
        m_instance = this;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the instance.
    //! \return Return nullptr if real function was selected or the pointer to
    //! this instance (this) if the mock was selected.
    //--------------------------------------------------------------------------
    static ReadMock* instance()
    {
        return m_instance;
    }

    //--------------------------------------------------------------------------
    //! \brief Google mock entry point.
    //--------------------------------------------------------------------------
    MOCK_METHOD3(read, ssize_t(int fd, char* buf, size_t nbytes));

    //! Init to nullptr inside the mocks.cpp to force loading the real symbold
    //! of the read function if the constructor of this method is not called.
    static ReadMock* m_instance;
};

//******************************************************************************
// Mock the C function open(). Idem than ReadMock
//******************************************************************************
class OpenMock
{
public:

    OpenMock()
    {
        m_instance = this;
    }

    static OpenMock* instance()
    {
        return m_instance;
    }

    MOCK_METHOD2(open, int(const char* path, int flag));

    static OpenMock* m_instance;
};
