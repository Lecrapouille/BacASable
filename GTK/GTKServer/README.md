# Trying GTK-server and GTK+2

These are very basic examples of GTK+2 (containing deprecated GTK+2 routines) for testing GTK-server https://www.gtk-server.org/

Examples have been adapted from examples given within the GTK-server package.
They are developed quick and dirty and as-is. So they are very usable 'as-is'.

- gtk_drawing_area-not-working.bash: gtk drawing area (not working WIP)

- gtk_image-fifo.bash: (Linux) bash code showing a red rectangle + circle.
```sh
chmod +x gtk_image-fifo.bash
./gtk_image-fifo.bash
```

- gtk_image-tcp.bash: (Linux and Window10 + msys2) bash code showing a red rectangle + circle.
  - From Window10 prompt (or bat file):
```bat
gtk-server -tcp=localhost:50000 -detach
```

  - Or, Linux side:
```sh
gtk-server-gtk2 -tcp=127.0.0.1:50000
```

  - From Window10 msys2 prompt or second Linux console:
```sh
chmod +x gtk_image-tcp.bash
./gtk_image-fifo.bash
```

- gtk_image.gforth: (Linux) port of gtk_image-fifo.bash to gforth. You'll need to install gforth and https://github.com/irdvo/ffl. And you need to adapt the path to `gtk-server.cfg` in the line `s" gtk-server.cfg" s" /tmp/fifo" gsv+open 0= [IF]` and the line `fpath path= /home/qq/ffl/|./` to find ffl files.
  - In the first console:
```sh
gtk-server-gtk2 -fifo=/tmp/fifo
```

  - In the second console:
```sh
gforth
include gtk_image.gforth
```

- gtk_image-tcp.forthwin (Windows 10) port of gtk_image-fifo.bash to ForthWin.
  - From Window10 prompt (or bat file):
```bat
gtk-server -tcp=localhost:50000 -detach
```

  - From ForthWin:
```
S" c:\gtk_image-tcp.forthwin" INCLUDED
```

