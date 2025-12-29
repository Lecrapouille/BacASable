# Camera Intrinsic and Extrinsic Settings

Not code of mine! See:
- (en) https://learnopencv.com/camera-calibration-using-opencv

More links:
- (en) https://www.youtube.com/watch?v=hUVyDabn1Mg

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

## Compilation

Compilation for Linux or for OS X:
```
g++ -W -Wall --std=c++11 CameraSettings.cpp -o CameraSettings `pkg-config --libs --cflags opencv`
```

## Run

```
./CameraSettings
```
