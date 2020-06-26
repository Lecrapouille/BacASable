#!/bin/bash
#-------------------------------------------------------------------------------------------------

rm $HOME/.gtk4bash

# Pipe filename must be unique for your application
PIPE="/tmp/gtk.bash.\$$"

# Find GTK-server configfile first
if [[ -f gtk-server.cfg ]]; then
    CFG=gtk-server.cfg
elif [[ -f /etc/gtk-server.cfg ]]; then
    CFG=/etc/gtk-server.cfg
elif [[ -f /usr/local/etc/gtk-server.cfg ]]; then
    CFG=/usr/local/etc/gtk-server.cfg
else
    echo "No GTK-server configfile found! Please install GTK-server..."
    exit 1
fi
# Now create global functionnames from GTK API
if [[ ! -f $HOME/.gtk4bash || $CFG -nt $HOME/.gtk4bash ]]; then
    echo "#!/bin/bash" > $HOME/.gtk4bash
    echo "gtk-server-gtk2 -fifo=$PIPE &" >> $HOME/.gtk4bash
    echo "while [ ! -p $PIPE ]; do continue; done" >> $HOME/.gtk4bash
    while read LINE
    do
	if [[ $LINE = FUNCTION_NAME* ]]; then
	    LINE=${LINE#*= }
	    printf "\nfunction ${LINE%%,*}\n" >> $HOME/.gtk4bash
	    printf "{\n/bin/echo ${LINE%%,*} \$@ > $PIPE" >> $HOME/.gtk4bash
	    printf "\nread GTK < $PIPE\n}\n" >> $HOME/.gtk4bash
	fi
    done < $CFG
fi
# Declare global variables
declare GTK NULL="NULL"
unset CFG PIPE LINE

# Include the generated '.gtk4bash'-file
. $HOME/.gtk4bash

# Assignment function
function define() { $2 $3 $4 $5 $6 $7 $8 $9; eval $1="$GTK"; }

################################################################
# Init the window
gtk_init $NULL $NULL
define WIN gtk_window_new 0
gtk_window_set_title $WIN "'BASH Analog Clock'"
gtk_widget_set_size_request $WIN 300 350
gtk_window_set_position $WIN 1
gtk_window_set_resizable $WIN 0
gtk_window_set_icon_name $WIN "clock"
# Create widget to display image
define IMAGE gtk_image_new
gtk_container_add $WIN $IMAGE
# Show all widgets
gtk_widget_show_all $WIN

################################################################
# Create the pixmap
define GDKWIN gtk_widget_get_parent_window $IMAGE
define PIX gdk_pixmap_new $GDKWIN 300 350 -1
define GC gdk_gc_new $PIX
gtk_image_set_from_pixmap $IMAGE $PIX $NULL
define COLOR gtk_server_opaque

################################################################
# White window
gdk_color_parse "#ffffff" $COLOR
gdk_gc_set_rgb_bg_color $GC $COLOR
gdk_gc_set_rgb_fg_color $GC $COLOR
gdk_draw_rectangle $PIX $GC 1 0 0 300 350

################################################################
# Black circle
gdk_color_parse "#000000" $COLOR
gdk_gc_set_rgb_fg_color $GC $COLOR
gdk_draw_arc $PIX $GC 0 5 0 290 290 0 23000

################################################################
# Red rectangle
gdk_color_parse "#ff0000" $COLOR
gdk_gc_set_rgb_fg_color $GC $COLOR
gdk_draw_rectangle $PIX $GC 1 0 0 30 35

# Mainloop
EVENT=0
while [[ $EVENT -ne $WIN ]]
do
    # Get event
    define EVENT gtk_server_callback "wait"

done

# Exit GTK
gtk_server_exit
