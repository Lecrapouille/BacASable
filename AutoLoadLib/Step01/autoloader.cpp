#include <iostream>
#include <glibmm/module.h>

static const std::string c_livelib = "./liblivecode"; // .so or .dylib or .dll
static const std::string c_livefunc = "helloworld";
typedef void (*fptr)();

int main()
{
  Glib::Module module(c_livelib);
  if (module)
    {
      void* func = nullptr;
      bool found = module.get_symbol(c_livefunc, func);
      std::cout << "found the function "
                << c_livefunc << "() ? "
                << (found ? "yes" : "no") << std::endl;

      if (found)
        {
          fptr my_fptr = reinterpret_cast<fptr>(reinterpret_cast<long>(func)) ;
          my_fptr();
        }
    }
  else
    {
      std::cerr
        << "Error when loading '" << c_livelib << "'" << std::endl;
    }

  return 0;
}
