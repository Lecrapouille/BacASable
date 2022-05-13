#include <iostream>
#include <glibmm/module.h>

typedef void (*fptr)();

static Glib::Module *module = nullptr;
// The file name of the code which has to be compiled into c_livelib
static const std::string c_livefile = "lib.c";
// The shared lib to be auto-loaded (can be .so or .dylib or .dll)
static const std::string c_livelib = "./liblivecode";
// The function to load from c_livelib
static const std::string c_livefunc = "helloworld";
// Save the previous time where the c_livefile has been modified
static long prevUpdateTime = 0;

// Reload the live library c_livelib
static bool reload()
{
  bool ret = false; // failure

  // Don't forget to clean up the lib before loading it
  if (nullptr != module)
    {
      // std::cout << "delete module" << std::endl;
      delete module;
    }

  // Load the lib
  module = new Glib::Module(c_livelib);
  if ((nullptr != module) && (*module))
    {
      void* func = nullptr;
      if (module->get_symbol(c_livefunc, func))
        {
          fptr my_fptr = reinterpret_cast<fptr>(reinterpret_cast<long>(func));
          my_fptr();
          ret = true;
        }
      else
        {
          std::cerr << "Couldn't find the function "
                    << c_livefunc << "() '"
                    << module->get_last_error()
                    << "'" << std::endl;
        }
    }
  else
    {
      std::cerr << "Error: No dice loading '"
                << c_livelib << "'"
                << std::endl;
    }
  return ret;
}

#include <cstdlib> // system()

// Call the makefile and compile the live library c_livelib
static void recompileAndReload()
{
  std::string cmd = "make lib";
  system(cmd.c_str());
  reload();
}

#  include <sys/stat.h> // stat()

// Check if the live file lib.c has changed. If yes, reload it.
void checkAndUpdate(std::string const & livefile)
{
  struct stat fileStat;

  if (stat(livefile.c_str(), &fileStat) < 0)
    {
      std::cerr << "Couldn't stat file " << livefile << std::endl;
      return;
    }

  long currUpdateTime = fileStat.st_mtime;
  if (currUpdateTime != prevUpdateTime)
    {
      // std::cout << "#################################" << std::endl;
      // std::cout << "Compile reload" << std::endl;
      recompileAndReload();
    }
  prevUpdateTime = currUpdateTime;
}

#include <unistd.h> // sleep()

// Check every 100 ms if the lib has to be reloaded.
int main()
{
  std::cout << "Please edit '" << c_livefile
            << "' file with your favorite code editor.\n"
            << "Each time you will save it after a modification "
            << "this file will be compiled and the modification "
            << "will appear on the screen."
            << std::endl;
  while (true)
    {
      checkAndUpdate(c_livefile);
      usleep(100000);
    }

  return 0;
}
