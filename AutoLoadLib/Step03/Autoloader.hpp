// Quentin Quadrat quentin.quadrat@gmail.com
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <https://unlicense.org>

#include <dlfcn.h>
#include <functional>
#include <string>
#include <stdexcept>
#include <sys/stat.h>

// *****************************************************************************
//! \brief Class allowing to load C functions from a given shared library. This
//! class wraps C functions dlopen(), dlsym(), dlerror().
// *****************************************************************************
class IDynamicLoader
{
public:

    //--------------------------------------------------------------------------
    //! \brief
    //--------------------------------------------------------------------------
    enum Resolution { LAZY = RTLD_LAZY, NOW = RTLD_NOW };

    //--------------------------------------------------------------------------
    //! \brief Close the shared library.
    //! \note this will invalidate pointer of symbols you have extracted.
    //--------------------------------------------------------------------------
    virtual ~IDynamicLoader()
    {
        close();
    }

    //--------------------------------------------------------------------------
    //! \brief Load the given shared library (.so, .dylib, .dll file).
    //! Call the virtual method onLoading() in which you should extract symbols
    //! you want (ie with the method prototype()).
    //! \param[in] lib_name: the path of the shared library (.so, .dll, .dylib)
    //! to open.
    //! \param[in] rt load symbols immediatly or in lazy way.
    //--------------------------------------------------------------------------
    bool load(const char* lib_name, Resolution rt = Resolution::NOW)
    {
        m_path = lib_name;
        close();
        ::dlerror();
        m_handle = ::dlopen(lib_name, rt);
        if (m_handle == nullptr)
        {
            set_error(::dlerror());
            m_prevUpdateTime = 0;
        }
        else
        {
            try
            {
                onLoading();
                m_prevUpdateTime = getFileTime();
            }
            catch(std::logic_error &e)
            {
                return false;
            }
        }

        return m_handle != nullptr;
    }

    //--------------------------------------------------------------------------
    //! \brief Close the shared library and reload its symbols.
    //! \param[in] rt load symbols immediatly or in lazy way.
    //--------------------------------------------------------------------------
    bool reload(Resolution rt = Resolution::NOW)
    {
        return load(m_path.c_str(), rt);
    }

    //--------------------------------------------------------------------------
    //! \brief Call reload() if the time of the shared library has changed.
    //--------------------------------------------------------------------------
    bool reloadIfChanged(Resolution rt = Resolution::NOW)
    {
        long time = getFileTime();
        if (time == 0)
        {
            return false;
        }
        if (time != m_prevUpdateTime)
        {
            m_prevUpdateTime = time;
            return reload(rt);
        }

        return false;
    }

    //--------------------------------------------------------------------------
    //! \brief Check if the shared library has been opened.
    //! \return true if the shared library has been opened with success.
    //--------------------------------------------------------------------------
    inline operator bool() const
    {
        return m_handle != nullptr;
    }

    //--------------------------------------------------------------------------
    //! \brief Close the shared library (even if it was not loaded).
    //! \note this will invalidate pointer of symbols you have extracted.
    //--------------------------------------------------------------------------
    void close()
    {
        if (m_handle != nullptr)
        {
            ::dlclose(m_handle);
            m_handle = nullptr;
        }
        m_error.clear();
    }

    //--------------------------------------------------------------------------
    //! \brief Return the address of the symbol given its name.
    //! \return the address of the symbol if present, else return nullptr.
    //--------------------------------------------------------------------------
    void* address(const char* symbol)
    {
        if (m_handle != nullptr)
        {
            ::dlerror();
            void* addr = ::dlsym(m_handle, symbol);
            const char* error = ::dlerror();
            if ((addr == nullptr) && (error != nullptr))
            {
                set_error(error);
            }
            else
            {
                m_error.clear();
            }
            return addr;
        }
        else
        {
            set_error("shared library not opened!");
            return nullptr;
        }
    }

    //--------------------------------------------------------------------------
    //! \brief Similar to address() but return a std::function and throw an
    //! exception if the symbol was not find.
    //--------------------------------------------------------------------------
    template<typename T>
    std::function<T> prototype(const char* symbol)
    {
        void* addr = address(symbol);
        if (addr == nullptr)
        {
            //std::cerr << error() << std::endl;
            throw std::logic_error(error());
        }
        return reinterpret_cast<T*>(addr);
    }

    //--------------------------------------------------------------------------
    //! \brief Return the path of the shared library.
    //--------------------------------------------------------------------------
    inline std::string const& path() const
    {
        return m_path;
    }

    //--------------------------------------------------------------------------
    //! \brief Return the latest error.
    //--------------------------------------------------------------------------
    inline std::string const& error() const
    {
        return m_error;
    }

private:

    //--------------------------------------------------------------------------
    //! \brief Callback to implement when load() or reload() has ended with
    //! success.
    //--------------------------------------------------------------------------
    virtual void onLoading()
    {}

    //--------------------------------------------------------------------------
    //! \brief Concat error messages.
    //--------------------------------------------------------------------------
    void set_error(const char* msg)
    {
        if (msg == nullptr)
            return ;
        m_error += std::string("- ") + msg + '\n';
    }

    //--------------------------------------------------------------------------
    //! \brief Return the date of the shared library.
    //! \pre The shared library shall be loaded.
    //--------------------------------------------------------------------------
    long getFileTime()
    {
        struct stat fileStat;

        if (stat(m_path.c_str(), &fileStat) < 0)
        {
            set_error("Couldn't stat file");
            return 0;
        }

        return fileStat.st_mtime;
    }

private:

    //! \brief Memorize the path of the shared library.
    std::string m_path;
    //! \brief Memorize the latest error.
    std::string m_error;
    //! \brief The handle on the opened shared library.
    void *m_handle = nullptr;
    //! \brief The date of the shared lib
    long m_prevUpdateTime = 0;
};
