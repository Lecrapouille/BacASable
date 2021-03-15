#include <dlfcn.h>
#include <stdarg.h>
#include "mocks.hpp"

ReadMock* ReadMock::m_instance = nullptr;
OpenMock* OpenMock::m_instance = nullptr;

// extern "C" is mandatory
extern "C"
{

ssize_t read(int fd, void* buf, size_t count)
{
    if (ReadMock::instance())
    {
        // Call the mocked method

        std::cout << "Calling the mocked 'read' function" << std::endl;
        return ReadMock::instance()->read(fd, static_cast<char*>(buf), count);
    }
    else
    {
        // Load the symbol of the C function and call it.

        typedef int (*FuncPtr_t) (int fd, void* buf, size_t count);
        static FuncPtr_t funcPtr = (FuncPtr_t) dlsym(RTLD_NEXT, "read");
        if (funcPtr)
        {
            std::cout << "Calling the 'read' function from clib" << std::endl;
            return funcPtr(fd, buf, count);
        }

        ADD_FAILURE() << __func__ << " failed loading symbol 'read'";
        return -1;
    }
}

int open(const char * pathname, int flags, ...)
{
    if (OpenMock::instance())
    {
        std::cout << "Calling the mocked 'open' function" << std::endl;
        return OpenMock::instance()->open(pathname, flags);
    }
    else
    {
        typedef int (*FuncPtr_t) (const char * __restrict__ pathname, int flags, ...);
        static FuncPtr_t funcPtr = (FuncPtr_t) dlsym(RTLD_NEXT, "open");
        if (funcPtr)
        {
            std::cout << "Calling the 'open' function from clib" << std::endl;
            return funcPtr(pathname, flags);
        }

        ADD_FAILURE() << __func__ << " failed loading symbol 'open'";
        return -1;
    }
}

} // extern "C"
