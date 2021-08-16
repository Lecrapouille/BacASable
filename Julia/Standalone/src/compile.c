#include <string.h>
#include <stdint.h>

// Julia headers
#include "uv.h"
#include "julia.h"
#include <gtk/gtk.h>

JULIA_DEFINE_FAST_TLS()

int julia_main();

int main(int argc, char *argv[])
{
   uv_setup_args(argc, argv);
   libsupport_init();
   jl_options.image_file = JULIAC_PROGRAM_LIBNAME;
   julia_init(JL_IMAGE_JULIA_HOME);
   jl_set_ARGS(argc, argv);
   jl_set_global(jl_base_module,jl_symbol("PROGRAM_FILE"), (jl_value_t*)jl_cstr_to_string(argv[0]));
   jl_array_t *ARGS = (jl_array_t*)jl_get_global(jl_base_module, jl_symbol("ARGS"));
   jl_array_grow_end(ARGS, argc - 1);
   for (int i = 1; i < argc; i++)
   {
      jl_value_t *s = (jl_value_t*)jl_cstr_to_string(argv[i]);
      jl_arrayset(ARGS, s, i - 1);
   }

   int ret = julia_main();
   jl_atexit_hook(ret);

   return ret;
}
