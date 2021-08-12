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
//! \brief Polynomial fitting
//******************************************************************************
class PolyFit
{
public:
    bool init(std::vector<double> const& vx, std::vector<double> const&vy)
    {
        assert(vx.size() == vy.size());

        m_coefs.clear();
        if (vx.size() == 0u)
        {
            m_coefs.push_back(0);
            m_coefs.push_back(0);
            return m_init = false;
        }

        // Convert std::vector to cv::Mat
        cv::Mat x = cv::Mat(int(vx.size()), 1, CV_64F, (void*) vx.data());
        cv::Mat y = cv::Mat(int(vy.size()), 1, CV_64F, (void*) vy.data());
        cv::Mat z;

        // Opencv polyfit:
        // https://forum.opencv.org/t/matlab-polyfit-in-opencv/2474/2
        cv::Mat xx = cv::Mat::ones(x.rows, 2, CV_64F);
        x.copyTo(xx(cv::Range(0, x.rows), cv::Range(0, 1)));
        cv::SVD s(xx);
        s.backSubst(y, z);

        // Copy poly coefs
        double *ptr = z.ptr<double>(0);
        std::vector<double> dest(ptr, ptr + z.rows);
        m_coefs = dest;

        return m_init = true;
    }

    operator bool() const
    {
        return m_init;
    }

    double operator()(double x) const
    {
        return m_coefs[1] + m_coefs[0] * x;
    }

private:
    bool m_init = false;
    std::vector<double> m_coefs;
};

//******************************************************************************
// Help the Hough algorithm to find the road lanes by masking the environement
//******************************************************************************
static void MaskForHough(cv::Mat& img, cv::Mat& masked, cv::Mat& debug)
{
    // Section of the road we keep for Hough algorithm
    std::vector<cv::Point2i> vertices1 = {
        cv::Point2i(0, 520),
        cv::Point2i(750, 520),
        cv::Point2i(536, 422),
        cv::Point2i(288, 422),
        cv::Point2i(0, 484),
    };

    // Section of the road we discard
    std::vector<cv::Point2i> vertices2 = {
        cv::Point2i(146, 540),
        cv::Point2i(420, 540),
        cv::Point2i(424, 414),
        cv::Point2i(146, 540),
    };

    // Create the mask
    cv::Mat mask = cv::Mat::zeros(img.size(), img.type());
    cv::fillConvexPoly(mask, vertices1, cv::Scalar(255));
    cv::fillConvexPoly(mask, vertices2, cv::Scalar(0));
    cv::bitwise_and(img, mask, masked);

    // Debug
    cv::polylines(debug, vertices1, false, cv::Scalar(255,255,255), 3);
    cv::polylines(debug, vertices2, false, cv::Scalar(0,0,0), 3);
}

//******************************************************************************
// Compute the q-th quantile of norms of segment lines.
// Equivalent to np.quantile
// param[in] q Quantile or sequence of quantiles to compute, which must be
// between 0 and 100 inclusive
//******************************************************************************
static float getLengthThreshold(std::vector<cv::Vec4i> const& lines, size_t q)
{
    std::vector<float> v;
    v.reserve(lines.size());
    for (auto const& l: lines)
    {
        // normof the segment line without the std::sqrt
        float sqnorm = ((l[2] - l[0]) * (l[2] - l[0])) + ((l[3] - l[1]) * (l[3] - l[1]));
        v.push_back(sqnorm);
    }
    std::sort(v.begin(), v.end());
    return v[v.size() * q / 100u];
}

//******************************************************************************
// From multiple detected Hough lines return the polyfit of border lanes
//******************************************************************************
static void polyfitLanes(std::vector<cv::Vec4i> const& lines,
                         PolyFit& right_lane, PolyFit& left_lane)
{
    const float LENGTH_THRESHOLD = getLengthThreshold(lines, 30u);

    std::vector<double> left_x;
    std::vector<double> left_y;
    std::vector<double> right_x;
    std::vector<double> right_y;

    for (auto const& l: lines)
    {
        if (l[0] == l[1])
            continue;

        float x1 = l[0];
        float y1 = l[1];
        float x2 = l[2];
        float y2 = l[3];

        // in code, y is positive down
        float slope = (y1 - y2) / (x2 - x1);
        float sqnorm = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));

        // Ensure only long lines are considered
        if (sqnorm < LENGTH_THRESHOLD)
            continue;

        // these coords belong to right line
        if (slope < 0.0f)
        {
            right_x.push_back(x1); right_x.push_back(x2);
            right_y.push_back(y1); right_y.push_back(y2);
        }
        else // left line
        {
            left_x.push_back(x1); left_x.push_back(x2);
            left_y.push_back(y1); left_y.push_back(y2);
        }
    }

    right_lane.init(right_y, right_x);
    left_lane.init(left_y, left_x);
}

//******************************************************************************
static void detectLanes(cv::Mat& imgCanny, PolyFit& right_lane, PolyFit& left_lane, cv::Mat& debug)
{
    std::vector<cv::Vec4i> raw_lines;
    cv::Mat imgHough;

    MaskForHough(imgCanny, imgHough, debug);
    cv::HoughLinesP(imgHough, raw_lines, 1, CV_PI/180, 20, 30, 10);
    polyfitLanes(raw_lines, right_lane, left_lane);
}

//******************************************************************************
static void drawDetectedLane(cv::Mat& frame, PolyFit& lane, cv::Scalar color)
{
    const int BOTTOM_Y = 425;
    const int TOP_Y = 630;

    // No lane found
    if (!lane) {
        return ;
    }

    int x_start = int(lane(BOTTOM_Y));
    int x_end = int(lane(TOP_Y));
    cv::line(frame, cv::Point(x_start, BOTTOM_Y), cv::Point(x_end, TOP_Y),
             color, 5, cv::LINE_AA);
}

//******************************************************************************
static void drawHoughLines(cv::Mat& frame, std::vector<cv::Vec4i> const& lines)
{
    for (auto const& l: lines)
    {
        cv::Point p1(l[0], l[1]);
        cv::Point p2(l[2], l[3]);
        cv::line(frame, p1, p2, cv::Scalar(0,255,0), 3, cv::LINE_AA);
        cv::circle(frame, p1, 5, cv::Scalar(0,0,255), cv::FILLED);
        cv::circle(frame, p2, 5, cv::Scalar(0,0,255), cv::FILLED);
    }
}

//******************************************************************************
static void crossingLaneAssistant(cv::Mat& frame, PolyFit const& left_lane)
{
    double const ly = 450.0; // left lane y coordinate
    double const center = 250.0; // Ideal center of the left lane
    double const margin = 50.0; // left lane security margin (width)

    // Show the security margin
    cv::line(frame, cv::Point(center - margin, ly),
                    cv::Point(center + margin, ly), cv::Scalar(255,0,0), 5, cv::LINE_AA);

    // Get the X-coordinate of the line
    double const lx = left_lane(ly);
    cv::circle(frame, cv::Point2f(lx, ly), 8, cv::Scalar(0,0,255), cv::FILLED);

    // Draw the directional arrow indicating to the driver in which direction he has to turn
    double delta = lx - center;
    if (std::abs(delta) > margin / 2.0)
    {
        if (delta >= 0.0)
           cv::arrowedLine(frame, cv::Point2f(lx, ly), cv::Point2f(500, ly), cv::Scalar(0,0,255), 5);
        else
           cv::arrowedLine(frame, cv::Point2f(500, ly), cv::Point2f(lx, ly), cv::Scalar(0,0,255), 5);
    }
}

//******************************************************************************
// g++ -W -Wall --std=c++11 Franchissement.cpp -o Franchissement `pkg-config --libs --cflags opencv`
//******************************************************************************
int main()
{
    cv::Mat imgUnscaled, frame, imgGray, imgBlur, imgCanny;
    PolyFit right_lane, left_lane;

    // Open the video of driving cars on a highway.
    // Download this video in mp4: https://youtu.be/6q5_A5wOwDM
    cv::VideoCapture cap("autoroute.mp4");
    if (!cap.isOpened())
    {
        std::cerr << "Error opening video file" << std::endl;
        return EXIT_FAILURE;
    }

    while (true)
    {
        cap.set(cv::CAP_PROP_POS_FRAMES, 0); // Rewind video
        size_t i = cap.get(cv::CAP_PROP_FRAME_COUNT);
        while (i--)
        {
            cap.read(imgUnscaled);

            // Do some filtering treatments
            cv::resize(imgUnscaled, frame, cv::Size(), 0.75, 0.75);
            cv::cvtColor(frame, imgGray, cv::COLOR_BGR2GRAY);
            cv::GaussianBlur(imgGray, imgBlur, cv::Size(5,5), 5, 0);
            cv::Canny(imgBlur, imgCanny, 75, 150);

            // Find lanes using Hough and find the averaged lane (polyfit)
            detectLanes(imgCanny, right_lane, left_lane, frame);
            drawDetectedLane(frame, right_lane, cv::Scalar(0,255,0));
            drawDetectedLane(frame, left_lane, cv::Scalar(0,255,0));

            // Is car centered on its lane ?
            crossingLaneAssistant(frame, left_lane);

            imshow("Highway", frame);
            char c = cv::waitKey(1);
            if (c == 'q')
            {
                return EXIT_SUCCESS;
            }

            // TODO: implement a real pause
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    return EXIT_SUCCESS;
}
