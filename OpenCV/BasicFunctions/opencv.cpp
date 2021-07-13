#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

// OpenCV Examples shown in https://youtu.be/2FYm3GOonhk

// Step 1
static void basic_functions()
{
   cv::Mat img = cv::imread("belote.jpg");
   cv::imshow("Image Origin", img);

   cv::Mat imgGray;
   cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);
   cv::imshow("Image Grey", imgGray);

   cv::Mat imgBlur;
   cv::GaussianBlur(img, imgBlur, cv::Size(7,7), 5, 0);
   cv::imshow("Image Blur", imgBlur);

   cv::Mat imgCanny;
   cv::Canny(imgBlur, imgCanny, 50, 150);
   cv::imshow("Image Canny", imgCanny);

   cv::Mat imgDilation, imgErode;
   cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
   cv::dilate(imgCanny, imgDilation, kernel);
   cv::erode(imgDilation, imgErode, kernel);
   cv::imshow("Image Dilation", imgDilation);
   cv::imshow("Image Erode", imgErode);
}

// Step 2
static void basic_resize()
{
   cv::Mat img = cv::imread("belote.jpg");
   std::cout << "Size before " << img.size() << std::endl;
   cv::imshow("Image Origin", img);

   cv::Mat imgResize;
   // cv::resize(img, imgResize, cv::Size(200, 200));
   cv::resize(img, imgResize, cv::Size(), 0.5, 0.5);
   std::cout << "Size after " << imgResize.size() << std::endl;
   cv::imshow("Image resized", imgResize);

   cv::Mat imgCrop = img(cv::Rect(200, 200, 100, 100));
   std::cout << "Size crpped " << imgCrop.size() << std::endl;
   cv::imshow("Image cropped", imgCrop);
}

// Step 3
static void draw_shapes()
{
   cv::Mat img(cv::Size(512, 512), CV_8UC3, cv::Scalar(255, 0, 0)); // uint8_t, 3 color channels, blue
   cv::circle(img, cv::Point(256, 256), 155, cv::Scalar(0,69,255), 10); // cv::FILLED
   cv::rectangle(img, cv::Point(130,200), cv::Point(382,286), cv::Scalar(0,69,255), cv::FILLED);
   cv::line(img, cv::Point(130, 296), cv::Point(380,300), cv::Scalar(0, 0, 0), 20);
   cv::putText(img, "Hello",cv::Point(137,262), cv::FONT_HERSHEY_DUPLEX, 0.75, cv::Scalar(0, 0, 0), 2);

   cv::imshow("Image Origin", img);
}

// Step 4
static void warp_perspective()
{
   // Corners of the card (used Gimp to get it)
   cv::Point2f src[4] = {
      { 188, 220 }, { 270, 207 }, { 296, 303 }, { 210, 314 },
   };

   // Destination coordinates of the corner
   const float w = 250.0f; const float h = 350.0f;
   cv::Point2f dst[4] = {
      { 0.0f, 0.0f }, { w, 0.0f }, { w, h }, { 0.0f, h },
   };

   cv::Mat img = cv::imread("belote.jpg");
   size_t i = 4u;
   while (i--) {
      cv::circle(img, src[i], 4, cv::Scalar(255,0,0), cv::FILLED);
   }
   cv::imshow("Image Origin", img);

   cv::Mat imgUnwarp;
   cv::Mat matrix = cv::getPerspectiveTransform(src, dst);
   cv::warpPerspective(img, imgUnwarp, matrix, cv::Point(w,h));
   cv::imshow("Image Perspective", imgUnwarp);
   cv::imwrite("unwarp.jpg", imgUnwarp);
}

// Step 5
static void color_detection()
{
   cv::Mat img = cv::imread("circles.png");
   cv::resize(img, img, cv::Size(), 0.25, 0.25);
   cv::Mat imgHSV;
   cv::cvtColor(img, imgHSV, cv::COLOR_BGR2HSV);

   int hmin = 0; int smin = 110; int vmin = 153;
   int hmax = 19; int smax = 240; int vmax = 255;

   cv::namedWindow("Trackbars", 640);
   cv::createTrackbar("Hue Min", "Trackbars", &hmin, 179);
   cv::createTrackbar("Hue Max", "Trackbars", &hmax, 179);
   cv::createTrackbar("Sat Min", "Trackbars", &smin, 255);
   cv::createTrackbar("Sat Max", "Trackbars", &smax, 255);
   cv::createTrackbar("Val Min", "Trackbars", &vmin, 255);
   cv::createTrackbar("Val Max", "Trackbars", &vmax, 255);

   while (true)
   {
      cv::Mat mask;
      cv::Scalar lower(hmin, smin, vmin);
      cv::Scalar upper(hmax, smax, vmax);
      cv::inRange(imgHSV, lower, upper, mask);

      cv::imshow("Image Origin", img);
      cv::imshow("Image HSV", imgHSV);
      cv::imshow("mask", mask);
      cv::waitKey(1);
   }
}

// g++ -W -Wall --std=c++11 opencv.cpp -o opencv `pkg-config --libs --cflags opencv`
int main()
{
   // Uncomment one of the followin function
   // basic_functions();
   // basic_resize();
   // draw_shapes();
   // warp_perspective();
    color_detection();

   cv::waitKey(0);
   return 0;
}
