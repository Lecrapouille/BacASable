// Nom des fonctions C qui doivent etre exportees dans des blocs Scicos
comp_fun_lst=['scicos_camerafirewire', 'scicos_saveRGBimage'];

// Liste des fichiers C a compiler
c_prog_lst=['camera_scicos.c', 'camera_firewire.c', 'rw_image.c', 'trt_image.c', 'histogramme.c'];

// Leur equivalent compiles
o_prog_lst=strsubst(c_prog_lst, '.c', '.o');

// Bibliotheques paratgees (*.so)
shr_lib_lst= ['/usr/lib/libraw1394','/usr/lib/libdc1394_control'];

// Compilation
ilib_for_link(comp_fun_lst, o_prog_lst, shr_lib_lst, 'c');
genlib('lib_firewire', pwd());
