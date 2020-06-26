#!/bin/bash
#------------------------------------------------

# Define SIGUSR1 in configfile to catch the signal
trap 'exit' SIGUSR1

# Name of PIPE file
declare PIPE=/tmp/bash.gtk.$$
declare GTK NULL="NULL"

# Communicate with GTK-server
function gtk() { echo $@ > $PIPE; read RESULT < $PIPE; }
function define() { $2 "$3"; eval $1=$GTK; }

#------------------------ Main starts here

# Start gtk-server
gtk-server-gtk2 -fifo=$PIPE -log=/tmp/gtk-server.log &

# Make sure the PIPE file is available
while [ ! -p $PIPE ]; do continue; done

# Setup GUI
gtk "gtk_init $NULL $NULL"
gtk "gtk_window_new 0"
WIN=$RESULT
gtk "gtk_window_set_title $WIN 'Draw me a rectangle'"
gtk "gtk_window_set_position $WIN 1"
gtk "gtk_drawing_area_new"
AREA=$RESULT
gtk "gtk_container_add $WIN $AREA"
gtk "gtk_widget_set_usize $AREA 400 200"
gtk "gtk_drawing_area_size $AREA 400 200"
gtk "gtk_widget_add_events $AREA GDK_BUTTON_PRESS_MASK"
gtk "gtk_server_connect $AREA 'configure_event' 'drawing_area_configure_event'"
gtk "gtk_server_connect $AREA 'expose_event' 'drawing_area_expose_event'"
gtk "gtk_server_connect $AREA 'event' 'drawing_area_button_press_event'"
gtk "gtk_widget_show_all $WIN"

EVENT=0
PIXMAP=
GC=
COLOR=

# Mainloop
while [[ $EVENT -ne $WIN ]]
do
    gtk "gtk_server_callback WAIT"
    EVENT=$RESULT

    case $EVENT in

	"drawing_area_configure_event")
           echo "Configure"
           gtk "gtk_widget_get_parent_window $AREA"
           W=$RESULT
           gtk "gdk_pixmap_new $W 400 200 -1"
           PIXMAP=$RESULT
           gtk "gdk_gc_new $PIXMAP"
           GC=$RESULT
           gtk "gtk_server_opaque"
           COLOR=$RESULT
           gtk "gdk_color_parse '#00ffff' $COLOR"
           gtk "gdk_gc_set_rgb_bg_color $GC $COLOR"
           gtk "gdk_gc_set_rgb_fg_color $GC $COLOR"
           gtk "gdk_draw_rectangle $PIXMAP $GC 1 0 0 400 200"
	;;

	"drawing_area_expose_event")
           echo "Exposed"
           gtk "gtk_widget_get_parent_window $AREA"
           W=$RESULT
           gtk "gdk_draw_drawable $W $GC $PIXMAP 0 0 0 0 400 200"
	;;

        "drawing_area_button_press_event")
           echo "press"
           gtk "gdk_color_parse '#0000ff' $COLOR"
           gtk "gdk_gc_set_rgb_fg_color $GC $COLOR"
           gtk "gdk_draw_rectangle $PIXMAP $GC 1 0 0 400 200"

#err=
#           gtk "gdk_pixbuf_save $PIXMAP '/home/qq/foo.bmp' 'bmp'"
#           B=$RESULT
#echo " err $B $err"

           gtk "gtk_widget_queue_draw $AREA"
	;;

	*)
           echo "other event"
	;;
    esac
done

# Exit GTK
gtk "gtk_server_exit"
