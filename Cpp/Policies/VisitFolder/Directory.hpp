#ifndef DIRECTORY_HPP
#  define DIRECTORY_HPP

#include "Deleter.hpp"

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

//******************************************************************************
//! \brief Create the whole path like "mkdir -p"
//******************************************************************************
bool createDirectories(std::string const& path);

//******************************************************************************
//! \brief Check if the path is an existing folder.
//******************************************************************************
bool isValidDir(const char* path);

//******************************************************************************
//! \brief Create and open a file given a path. Directories if not present are
//! created.
//******************************************************************************
FileUP createFile(std::string const& path);

//******************************************************************************
//! \brief
//******************************************************************************
std::string removeRootPath(std::string const& mainString,
                           std::string const& toErase);

//******************************************************************************
//! \brief Class traversing recursively a directory, visiting elements through
//! traversal policies.
//! \tparam TraversalPolicy A policy class called on each elements of the dir.
//!
//! Example 1: The following code pretty print the content of a given directory
//! \code
//! class PrettyPrinter
//! {
//! public:
//!     bool onDirDetected(const char* basename, const char* filepath, int const level) {
//!         printf("%*s[%s]\n", 2 * level, "", basename);
//!         return true; // true for continuing traversing the directory
//!     }
//!
//!     bool onFileDetected(const char* basename, const char* dirpath, int const level) {
//!         printf("%*s- %s\n", 2 * level, "", basename);
//!         return true;  // true for continuing traversing the directory
//!     }
//! };
//!
//! DirectoryTraverser<PrettyPrinter> d;
//! d("/home/JohnDoe/Desktop");
//! \endcode
//!
//! Example 2: The following code find a file and halt on the first found occurence.
//! \code
//! class Finder
//! {
//! public:
//!     void lookfor(const char* file) {
//!         m_lookfor = file;
//!     }
//!
//!     bool onFileDetected(const char* basename, const char* filepath, int const level) {
//!         if (std::string(basename) == m_lookfor) {
//!             std::cout << "*** FOUND " << filepath << std::endl;
//!             return false;
//!         }
//!         return true;
//!     }
//!
//!     bool onDirDetected(const char* basename, const char* dirpath, int const level) {
//!         return true;
//!     }
//!
//! private:
//!     std::string m_lookfor;
//! };
//!
//! DirectoryTraverser<Finder> d;
//! d.lookfor("main.cpp");
//! d("/home/JohnDoe/Desktop");
//! \endcode
//******************************************************************************
template<class TraversalPolicy>
class DirectoryTraverser : public TraversalPolicy
{
public:

    //! ------------------------------------------------------------------------
    //! \brief Start traversing the directory refered by the given path.
    //! \param[in] dirpath the full path of the directory.
    //! \param[in] dirprefix optional, prefix for files/dirs in dirpath to be considered
    //! ------------------------------------------------------------------------
    void operator()(const char* dirpath, const char* dirprefix = nullptr)
    {
        bool abort = false;
        traverse(dirpath, 0, abort, dirprefix);
    }

private:

    //! ------------------------------------------------------------------------
    //! \brief Recursive method traversing the given directory.
    //! \param[in] dirpath the full path of the directory.
    //! \param[in] level the current depth in the hierarchy.
    //! \param[inout] abort for halting the traversal.
    //! \param[in] dirprefix optional, prefix for files/dirs in dirpath to be considered
    //! ------------------------------------------------------------------------
    void traverse(const char* dirpath, int const level, bool& abort, const char* dirprefix = nullptr)
    {
        DirUP dir;
        struct dirent *entry;
        size_t prefixlen = 0;

        dir.reset(opendir(dirpath));
        if (dir.get() == nullptr)
            return ;

        if (dirprefix)
        {
            prefixlen = strlen(dirprefix);
        }

        while ((!abort) && (entry = readdir(dir.get())) != nullptr)
        {
            if (dirprefix && strncmp(entry->d_name, dirprefix, prefixlen) != 0)
            {
                continue;
            }

            // Construct the full path
            std::string path(dirpath);
            path.append("/").append(entry->d_name);

            if (entry->d_type == DT_DIR)
            {
                if ((strcmp(entry->d_name, ".") == 0) ||
                    (strcmp(entry->d_name, "..") == 0))
                {
                    continue;
                }

                if (!TraversalPolicy::onDirDetected(entry->d_name, path.c_str(), level))
                {
                    abort = true;
                    break;
                }

                traverse(path.c_str(), level + 1, abort, nullptr);
            }
            else if ((entry->d_type == DT_REG) || (entry->d_type == DT_LNK))
            {
                if (!TraversalPolicy::onFileDetected(entry->d_name, path.c_str(), level))
                {
                    abort = true;
                    break;
                }
            }
        }
    }
};

#endif
