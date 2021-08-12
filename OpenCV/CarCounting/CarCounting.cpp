// 2021 Quentin Quadrat lecrapouille@gmail.com
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <https://unlicense.org>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

//******************************************************************************
//! \brief Return the median value from a vector of data.
//! \note The vector \c elements will be sorted (side effect).
//******************************************************************************
static int median(std::vector<int>& elements)
{
    std::nth_element(elements.begin(),
                     elements.begin() + elements.size() / 2u,
                     elements.end());
    return elements[elements.size() / 2u];
}

//******************************************************************************
//! \brief From a set of image return the resulting image holding median values
//! of pixels.
//! \param[in] vec vector of image. The size of the vector depends on the video
//! duration.
//! \return the median image.
//******************************************************************************
static cv::Mat block_median(std::vector<cv::Mat>& vec)
{
    cv::Mat medianImg(vec[0].rows, vec[0].cols, CV_8UC3, cv::Scalar(0, 0, 0));

    for (int row = 0u; row < vec[0].rows; ++row)
    {
        for (int col = 0u; col < vec[0].cols; ++col)
        {
            std::vector<int> elements_B;
            std::vector<int> elements_G;
            std::vector<int> elements_R;

            for (size_t imgNumber = 0u; imgNumber < vec.size(); ++imgNumber)
            {
                int B = vec[imgNumber].at<cv::Vec3b>(row, col)[0];
                int G = vec[imgNumber].at<cv::Vec3b>(row, col)[1];
                int R = vec[imgNumber].at<cv::Vec3b>(row, col)[2];

                elements_B.push_back(B);
                elements_G.push_back(G);
                elements_R.push_back(R);
            }

            medianImg.at<cv::Vec3b>(row, col)[0] = median(elements_B);
            medianImg.at<cv::Vec3b>(row, col)[1] = median(elements_G);
            medianImg.at<cv::Vec3b>(row, col)[2] = median(elements_R);
        }
    }

    return medianImg;
}

//******************************************************************************
//! \brief Remove dynamic objects on a video and return the static image. The
//! camera shall have a fixed position. Algorithm: For each pixel of the video,
//! we keep the median value along the whole video duration.
//!
//! To preserve the memory, each image (640x360) of the video is split into
//! several blocks (80x90) and stored on a vector. The size of this vector
//! depends on the video duration. Then, for each pixel of blocks stored in this
//! vector, we keep the median value of each pixel and we recreate a new block
//! of image. This new image is then stored at the correct location in the final
//! image that we name the background image.
//!
//! \param cap the video to filter.
//! \return the static image where dynamic objects have been removed.
//! \note we can of course save some CPU computations by converting the image
//! directly in grey and not to compute median on the whole video.
//******************************************************************************
static cv::Mat background_estimation(cv::VideoCapture& cap)
{
    cv::Mat frame, cropped;
    size_t const S = cap.get(cv::CAP_PROP_FRAME_COUNT);  // number of frames
    size_t const W = cap.get(cv::CAP_PROP_FRAME_WIDTH);  // 640
    size_t const H = cap.get(cv::CAP_PROP_FRAME_HEIGHT); // 360
    size_t const BX = 80u; // Width of the block
    size_t const BY = 90u; // Height of the block

    std::vector<cv::Mat> blocks(S);
    cv::Mat result(H, W, CV_8UC3, cv::Scalar(0, 0, 0));

    for (size_t bx = 0u; bx < W; bx += BX)
    {
        for (size_t by = 0u; by < H; by += BY)
        {
            cap.set(cv::CAP_PROP_POS_FRAMES, 0); // Rewind video
            for (size_t i = 0u; i < S; ++i)
            {
                cap.read(frame);
                blocks[i] = frame(cv::Rect(bx, by, BX, BY)).clone();
                blocks[i].copyTo(result(cv::Rect(bx, by, BX, BY)));
                cv::imshow("Background", result);
                cv::waitKey(1);
            }

            block_median(blocks).copyTo(result(cv::Rect(bx, by, BX, BY)));
            cv::imshow("Background", result);
            cv::waitKey(1);
        }
    }

    return result;
}

//******************************************************************************
//! \brief Create a mask image by removing the background image on the current
//! image of the video. For each pixel at position (x,x) we do:
//! Mask(x,y) = (abs(DynamicImage(x,y) - StaticImage(x,y)) > threshold) ? 255 : 0
//! \param[in] img: dyanmic image (the current image of the video).
//! \param[in] bg: static image (background)
//! \param[in] threshold: value in where pixel of the mask will be 1.
//! \return The mask image.
//******************************************************************************
static cv::Mat create_mask(cv::Mat const& img, cv::Mat const& bg, uint8_t const threshold)
{
    cv::Mat image;
    cv::cvtColor(img, image, cv::COLOR_BGR2GRAY);

    cv::Mat background;
    cv::cvtColor(bg, background, cv::COLOR_BGR2GRAY);

    cv::Mat mask(image.size(), CV_8UC1, cv::Scalar(0));
    for (int i = 0; i < image.cols; ++i)
    {
        for (int j = 0; j < image.rows; ++j)
        {
            if (abs(background.at<uchar>(j, i) - image.at<uchar>(j, i)) > threshold)
            {
                mask.at<uchar>(j, i) = 255u;
            }
        }
    }

    return mask;
}

//******************************************************************************
//! \brief Helper class defining a zone and count points inside
//******************************************************************************
class CountingZone
{
public:

    //! \brief Define the zone to watch. A blue rectangle will be draw.
    void zone(cv::Rect zone_)
    {
        m_zone = zone_;
    }

    //! \brief Is the zone contains the given point ?
    //! \return true if the point is inside the zone defined by this instance.
    bool contains(cv::Point2f const& pt)
    {
        return m_zone.contains(pt);
    }

    //! \brief Reset the counter.
    void begin()
    {
        m_count = 0u;
        m_color = cv::Scalar(255, 0, 0);
    }

    //! \brief Increment the counter.
    void incr()
    {
        ++m_count;
        m_color = cv::Scalar(0, 0, 255);
    }

    //! \brief Increment the number of car if the number of cars has increased
    //! compared the previous time.
    //! \note this basic algorithm is not 100% reliable.
    void end()
    {
        if (m_count > m_prev_count)
        {
            ++m_cars;
        }
        m_prev_count = m_count;
    }

    //! \brief Draw the number the zone and the number of cars
    void draw(cv::Mat& frame)
    {
        cv::Scalar color;
        cv::rectangle(frame, m_zone, m_color, 5);
        cv::putText(frame, std::to_string(m_cars),
                    cv::Point(m_zone.x, m_zone.y - 10),
                    cv::FONT_HERSHEY_COMPLEX_SMALL, 2,
                    cv::Scalar(255, 255, 255), 2);
        m_color = cv::Scalar(255, 0, 0);
    }

private:

    size_t m_cars = 0u;
    size_t m_count = 0u;
    size_t m_prev_count = 0u;
    cv::Rect m_zone;
    cv::Scalar m_color = cv::Scalar(255, 0, 0);
};

//******************************************************************************
//! \brief Detect dynamic objects and count them
//! \note We can save some CPU computations by reducing directly the mask to the
//! desired areas.
//******************************************************************************
static void detect_cars(cv::Mat& frame, cv::Mat const& mask, std::vector<CountingZone>& counters)
{
    // Do some treatments on the mask
    //cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
    //cv::Mat maskErode;
    //cv::erode(mask, maskErode, kernel);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
    cv::dilate(mask, mask, kernel, cv::Point(-1,-1), 3);
    cv::imshow("Mask", mask);

    // Find dynamic objects
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Show dynamic objects
    std::vector<cv::Point2f> centers(contours.size());
    std::vector<float> radius(contours.size());
    for (size_t i = 0; i < contours.size(); ++i)
    {
        cv::drawContours(frame, contours, int(i), cv::Scalar(255,0,0), 2, cv::LINE_8, hierarchy, 0);
        cv::minEnclosingCircle(contours[i], centers[i], radius[i]);
    }

    for (auto& counter: counters)
    {
        counter.begin();
        for (size_t i = 0u; i < contours.size(); ++i)
        {
            if ((radius[i] > 20.0f) && (counter.contains(centers[i])))
            {
                cv::circle(frame, centers[i], 5, cv::Scalar(0,0,255), 10);//cv::FILLED);
                counter.incr();
            }
        }
        counter.end();
    }
}

//******************************************************************************
// g++ -W -Wall --std=c++11 CarCounting.cpp -o CarCounting `pkg-config --libs --cflags opencv`
//******************************************************************************
int main()
{
    cv::Mat background, frame, mask;

    // Open the video of driving cars on a highway.
    cv::VideoCapture cap("autoroute.mp4");
    if (!cap.isOpened())
    {
        std::cerr << "Error opening video file" << std::endl;
        return EXIT_FAILURE;
    }
    //int rate = static_cast<int>(1000.0 / cap.get(vc::CAP_PROP_FPS));

    // Load the image of the highway if present, else create it.
    background = cv::imread("background.jpg");
    if (background.empty())
    {
        background = background_estimation(cap);
        cv::imwrite("background.jpg", background);
    }

    // Count driving cars on a 3-ways highway
    std::vector<CountingZone> counters(3);
    counters[0].zone(cv::Rect(cv::Point(75, 315), cv::Point(214, 360)));
    counters[1].zone(cv::Rect(cv::Point(215, 315), cv::Point(344, 360)));
    counters[2].zone(cv::Rect(cv::Point(345, 315), cv::Point(500, 360)));
    size_t i = cap.get(cv::CAP_PROP_FRAME_COUNT);
    while (i--)
    {
        cap.read(frame);
        mask = create_mask(frame, background, 50u);
        detect_cars(frame, mask, counters);

        for (auto& counter: counters)
        {
            counter.draw(frame);
        }
        imshow("Highway", frame);
        cv::waitKey(1);

        // TODO: implement a real pause
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return EXIT_SUCCESS;
}
