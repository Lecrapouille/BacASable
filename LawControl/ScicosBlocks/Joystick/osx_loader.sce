joystick_path=get_absolute_file_path('osx_loader.sce');
link('../../../Bibliotheque/Unix/Joystick/libJoystick_i386-Darwin.so');
link(joystick_path+'libScicosJoystick_i686-Linux.so',['scicos_joystick'],'c');