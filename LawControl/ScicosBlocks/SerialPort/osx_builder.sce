// Nom des fonctions C qui doivent etre exportees dans les blocs Scicos
comp_fun_lst=['serialport_scicos_printf', 'serialport_scicos_read', 'serialport_scicos_write', 'serialport_scicos_init', 'serialport_scicos_bootloader'];

// Liste des fichiers C a compiler
c_prog_lst=['Scicos_SerialPort.c'];
//Ou bien: c_prog_lst=listfiles('*.c');

// Leur equivalent compiles
o_prog_lst=strsubst(c_prog_lst, '.c', '.o');

// Bibliotheques (*.so, *.dylib) necessaires pour compiler cette lib
shr_lib_lst=['../../../Libraries/LibUnix/PhysicalLayer/Unix_File/libFile_i386-Darwin',
'../../../Libraries/LibUnix/PhysicalLayer/Unix_UART/libUART_i386-Darwin',
'../../../Libraries/LibCommon/BlackBox/libBlackBox_i386-Darwin',
'../../../Libraries/LibCommon/Collections/libCollections_i386-Darwin',
'../../../Libraries/LibCommon/DataLinkLayer/libDataLinkLayer_i386-Darwin',
'../../../Libraries/LibCommon/EasyTFTP/libTFTP_i386-Darwin'];

// Compilation
ilib_for_link(comp_fun_lst, o_prog_lst, shr_lib_lst, 'c');
