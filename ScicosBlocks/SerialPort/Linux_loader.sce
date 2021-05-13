serialport_read_path=get_absolute_file_path('Linux_loader.sce');

link('../../../Libraries/LibUnix/PhysicalLayer/Unix_File/libFile_i686-Linux.so');
link('../../../Libraries/LibUnix/PhysicalLayer/Unix_UART/libUART_i686-Linux.so');
link('../../../Libraries/LibCommon/BlackBox/libBlackBox_i686-Linux.so');
link('../../../Libraries/LibCommon/Collections/libCollections_i686-Linux.so');
link('../../../Libraries/LibCommon/DataLinkLayer/libDataLinkLayer_i686-Linux.so');
link('../../../Libraries/LibCommon/EasyTFTP/libTFTP_i386-Linux.so');

link(serialport_read_path+'libScicosSerialPort_i686-Linux.so',['serialport_scicos_printf','serialport_scicos_write','serialport_scicos_read','serialport_scicos_init','serialport_scicos_bootloader'],'c');
