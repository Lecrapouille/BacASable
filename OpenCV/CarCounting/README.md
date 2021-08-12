# Car counting with OpenCV

Download the highway mp4 video [at this link](https://github.com/L42Project/Tutoriels/blob/master/OpenCV/tutoriel13/autoroute.mp4) and place in this folder.

Application compilation for Linux or for OS X:
```
g++ -W -Wall --std=c++11 CarCounting.cpp -o CarCounting `pkg-config --libs --cflags opencv`
```

To run the application:
```
./CarCounting
```

Note: Remove the file `background.jpg` will force the application to create it back, but this will take some time to process.

Install OpenCV for Linux:
```
apt-get install libopencv-*-dev
```

Install OpenCV for OS X (if openCV has been installed with homebrew):
```
brew install opencv@3
export PKG_CONFIG_PATH="/usr/local/opt/opencv@3/lib/pkgconfig"
```

Based on French YT videos:
- https://youtu.be/ycpjZn2lcZg
- https://youtu.be/pkzT9MlICPE

TODO:
- https://youtu.be/O3b8lVF93jU
