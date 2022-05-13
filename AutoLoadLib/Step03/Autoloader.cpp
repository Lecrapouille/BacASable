#include "Autoloader.hpp"
#include <iostream>
#include <chrono>
#include <thread>

//-----------------------------------------------------------------------------
#define SHARED_LIB "./liblivecode.so"
#define LIVE_FILE "lib.cpp"

//-----------------------------------------------------------------------------
class DynamicLoader : public IDynamicLoader
{
public:

    // Load once the shared library and apply the loaded symbol.
    DynamicLoader()
    {
        if (!load(SHARED_LIB))
        {
            std::cerr << "Couldn't load " << SHARED_LIB << std::endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            std::cout << "Initial load: " << name() << std::endl;
        }
    }

    // Reload the symbole shared library if it has been modified.
    void update()
    {
        if (reloadIfChanged())
        {
            std::cout << "Reloaded: " << name() << std::endl;
        }
    }

private:

    // Load desired symbols.
    virtual void onLoading() override
    {
        name = prototype<const char* (void)>("simulation_name");
    }

    // Symbol to load
    std::function<const char* (void)> name = nullptr;
};

//-----------------------------------------------------------------------------
// Compile the C++ file if it has been modified (if its date has changed).
static void compile_live_file_if_modifed(const char* file)
{
    static long previous_time = 0;
    struct stat fileStat;

    if (stat(file, &fileStat) < 0)
    {
        std::cerr << "Couldn't stat file" << std::endl;
        return ;
    }

    long current_time = fileStat.st_mtime;
    if (current_time != previous_time)
    {
        previous_time = current_time;
        std::string cmd = "make lib";
        system(cmd.c_str());
    }
}

//-----------------------------------------------------------------------------
int main()
{
    DynamicLoader dll;

    std::cout << "Please edit '" << LIVE_FILE
              << "' file with your favorite code editor.\n"
              << "Each time you will save it after a modification "
              << "this file will be compiled and the modification "
              << "will appear on the screen."
              << std::endl;

    while (true)
    {
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(100ms);

        // Compile the shared lib if its source file has been modified
        compile_live_file_if_modifed(LIVE_FILE);

        // Reload the symbol
        dll.update();
    }

    return 0;
}
