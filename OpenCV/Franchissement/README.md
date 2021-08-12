# Highway lanes detection and crossing

Code based on these YT videos:
- (en) https://youtu.be/vCfEyrgRhY8 for lane detection
  - GitHub: https://github.com/techtribeyt/opencv-lane-detection/tree/main/part4
- (fr) https://youtu.be/bCElBNENCy4
  - GitHub: https://github.com/L42Project/Tutoriels/tree/master/OpenCV/tutoriel14

Possible improvements:
- (en) https://youtu.be/Gs5HlHKqAYQ
- (en) https://youtu.be/O3b8lVF93jU

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

Download this video as `autoroute.mp4` file in this folder:
- https://youtu.be/6q5_A5wOwDM

## Compilation

Compilation for Linux or for OS X:
```
g++ -W -Wall --std=c++11 Franchissement.cpp -o Franchissement `pkg-config --libs --cflags opencv`
```

## Run

```
./Franchissement
```
