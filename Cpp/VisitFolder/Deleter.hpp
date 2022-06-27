#ifndef DELETER_HPP
#  define DELETER_HPP

#  include <stdio.h>  // FILE*
#  include <dirent.h> // DIR*
#  include <memory>   // default_delete
#  include <unistd.h> // fsync

namespace std
{
    // -------------------------------------------------------------------------
    //! \brief Prefered using FILE* than iostream but still want to use RAI.
    // -------------------------------------------------------------------------
    template<>
    struct default_delete<FILE>
    {
        void operator()(FILE* file) const
        {
            if (file != nullptr)
                fclose(file);
        }
    };

    // -------------------------------------------------------------------------
    //! \brief Prefered using smart pointer when iterating recursively on
    //! directories.
    // -------------------------------------------------------------------------
    template<>
    struct default_delete<DIR>
    {
        void operator()(DIR* dir) const
        {
            if (dir != nullptr)
                closedir(dir);
        }
    };
} // namesapce std

using DirUP = std::unique_ptr<DIR>;
using FileUP = std::unique_ptr<FILE>;

#endif
