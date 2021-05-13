joystick_path=get_absolute_file_path('Linux_loader.sce');
link('../../../Bibliotheque/Unix/Joystick/libJoystick_i686-Linux.so');
link(joystick_path+'libScicosJoystick_i686-Linux.so',['scicos_joystick'],'c');