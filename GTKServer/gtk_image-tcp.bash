#!/bin/bash

IO=/dev/tcp/127.0.0.1/50000; exec 3<>$IO

################################################################
# Init window
echo -e "gtk_init $NULL $NULL" >&3; read -r tmp <&3
echo -e "gtk_window_new 0" >&3; read -r WIN <&3
echo -e "gtk_window_set_title $WIN \"BASH Analog Clock\"" >&3; read -r tmp <&3
echo -e "gtk_widget_set_size_request $WIN 300 350" >&3; read -r tmp <&3
echo -e "gtk_window_set_position $WIN 1" >&3; read -r tmp <&3
echo -e "gtk_window_set_resizable $WIN 0" >&3; read -r tmp <&3
echo -e "gtk_window_set_icon_name $WIN "clock"" >&3; read -r tmp <&3
echo -e "gtk_image_new" >&3; read -r IMAGE <&3
echo -e "gtk_container_add $WIN $IMAGE" >&3; read -r tmp <&3
echo -e "gtk_widget_show_all $WIN" >&3; read -r tmp <&3

################################################################
# Create the pixmap
echo -e "gtk_widget_get_parent_window $IMAGE" >&3; read -r GDKWIN <&3
echo -e "gdk_pixmap_new $GDKWIN 300 350 -1" >&3; read -r PIX <&3
echo -e "gdk_gc_new $PIX" >&3; read -r GC <&3
echo -e "gtk_image_set_from_pixmap $IMAGE $PIX $NULL" >&3; read -r tmp <&3
echo -e "gtk_server_opaque" >&3; read -r COLOR <&3

################################################################
# White window
echo -e "gdk_color_parse \"#ffffff\" $COLOR" >&3; read -r tmp <&3
echo -e "gdk_gc_set_rgb_bg_color $GC $COLOR" >&3; read -r tmp <&3
echo -e "gdk_gc_set_rgb_fg_color $GC $COLOR" >&3; read -r tmp <&3
echo -e "gdk_draw_rectangle $PIX $GC 1 0 0 300 350" >&3; read -r tmp <&3

################################################################
# Red rectangle
echo -e "gdk_color_parse \"#ff0000\" $COLOR" >&3; read -r tmp <&3
echo -e "gdk_gc_set_rgb_fg_color $GC $COLOR" >&3; read -r tmp <&3
echo -e "gdk_draw_rectangle $PIX $GC 1 0 0 30 35" >&3; read -r tmp <&3

################################################################
# Black circle
echo -e "gdk_color_parse \"#000000\" $COLOR" >&3; read -r tmp <&3
echo -e "gdk_gc_set_rgb_fg_color $GC $COLOR" >&3; read -r tmp <&3
echo -e "gdk_draw_arc $PIX $GC 0 5 0 290 290 0 23000" >&3; read -r tmp <&3

event="0"
while [ "$event" != "$win" ]; do
	echo -e "gtk_server_callback WAIT" >&3; read -r event <&3
done

echo -e "gtk_exit 0" >&3
echo "Enjoy the day..."
