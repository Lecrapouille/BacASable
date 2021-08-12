# Car Counting with OpenCV

Based on these YT videos:
- (fr) https://youtu.be/pkzT9MlICPE for the algorithm
  - GitHub: https://github.com/L42Project/Tutoriels
- (fr) https://youtu.be/ycpjZn2lcZg for estimating the background.

## Prerequisite

### OpenCV

Install OpenCV for Linux:
```
apt-get install libopencv-*-dev
```

Install OpenCV for OS X (if openCV has been installed with homebrew):
```
brew install opencv@3
export PKG_CONFIG_PATH="/usr/local/opt/opencv@3/lib/pkgconfig"
```

### The input video

Please download the highway `autoroute.mp4` video [at this link](https://github.com/L42Project/Tutoriels/blob/master/OpenCV/tutoriel13/autoroute.mp4) and place in this folder.

## Compilation

Compilation for Linux or for OS X:
```
g++ -W -Wall --std=c++11 CarCounting.cpp -o CarCounting `pkg-config --libs --cflags opencv`
```

## Run

```
./CarCounting
```

Note: Removing the file `background.jpg` will force the application to create it back (this will take some time to process).
