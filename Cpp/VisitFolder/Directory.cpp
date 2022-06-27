#include "Directory.hpp"
#include <boost/filesystem.hpp>
#include <stdio.h>

// -------------------------------------------------------------------------
bool createDirectories(std::string const& path)
{
    try
    {
        boost::filesystem::create_directories(path);
        return true;
    }
    catch (std::exception const& e)
    {
        printf("Failed creating directories '%s'. Reason was '%s'",
               path.c_str(), e.what());
        return false;
    }
}

// -------------------------------------------------------------------------
bool isValidDir(const char* path)
{
    DIR* dir = opendir(path);
    if (dir)
    {
        // Directory exists
        closedir(dir);
        return true;
    }
    else if (ENOENT == errno)
    {
        // Directory does not exist.
        return false;
    }
    else
    {
        // opendir() failed for some other reason.
        return false;
    }
}

// -------------------------------------------------------------------------
FileUP createFile(std::string const& path)
{
    std::string::size_type pos = path.find_last_of("\\/");
    if (pos != std::string::npos)
    {
        std::string dir = path.substr(0, pos);
        if (!createDirectories(dir))
            return nullptr;
    }

    FileUP fp(fopen(path.c_str(), "wb+"));
    return fp;
}

// -----------------------------------------------------------------------------
//! \note mainString is const because of FileInfo const& file
std::string removeRootPath(std::string const& mainString, std::string const& toErase)
{
    std::string mainStr(mainString);

    // Search for the substring in string
    size_t pos = mainStr.find(toErase);

    if (pos != std::string::npos)
    {
        // If found then erase it from string
        mainStr = mainStr.substr(pos + toErase.length(), std::string::npos);
    }

    return mainStr;
}
