// Nom des fonctions C qui doivent etre exportees dans les blocs Scicos
comp_fun_lst=['scicos_joystick'];

// Liste des fichiers C a compiler
c_prog_lst=['scicos_joystick.c'];
//Ou bien: c_prog_lst=listfiles('*.c');

// Leur equivalent compiles
o_prog_lst=strsubst(c_prog_lst, '.c', '.o');

// Bibliotheques (*.so, *.dylib) necessaires pour compiler cette lib
shr_lib_lst=['../../../Bibliotheque/Unix/Joystick/libJoystick_i686-Linux'];

// Compilation
ilib_for_link(comp_fun_lst, o_prog_lst, shr_lib_lst, 'c');
