#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

static int median(std::vector<int>& elements)
{
    std::nth_element(elements.begin(),
                     elements.begin() + elements.size() / 2u,
                     elements.end());
    return elements[elements.size() / 2u];
}

// Filtering using the median value pixel along the video duration
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

static cv::Mat background_estimation(cv::VideoCapture& cap)
{
    cv::Mat frame, cropped;
    size_t const S = cap.get(cv::CAP_PROP_FRAME_COUNT); // number of frames
    size_t const W = cap.get(cv::CAP_PROP_FRAME_WIDTH); // 640
    size_t const H = cap.get(cv::CAP_PROP_FRAME_HEIGHT); // 360
    size_t const BX = 80u;
    size_t const BY = 90u;

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

// Mask(x,y) = (abs(Image(x,y) - Background(x,y)) > threshold) ? 255 : 0
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

    //cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
    //cv::Mat maskErode;
    //cv::erode(mask, maskErode, kernel);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
    cv::dilate(mask, mask, kernel, cv::Point(-1,-1), 3);

    cv::imshow("Mask", mask);
    cv::waitKey(1);
    return mask;
}

// g++ -W -Wall --std=c++11 CarCounting.cpp -o CarCounting `pkg-config --libs --cflags opencv`
int main()
{
    cv::VideoCapture cap("autoroute.mp4");
    if (!cap.isOpened())
    {
        std::cerr << "Error opening video file" << std::endl;
        return EXIT_FAILURE;
    }

    cv::Mat background;
    background = cv::imread("background.jpg");
    if (background.empty())
    {
        background = background_estimation(cap);
        cv::imwrite("background.jpg", background);
    }

    // TODO: dirty to be clean !

    size_t cars = 0u;
    size_t count = 0u; size_t prev_count = 0u;
    uint8_t seuil = 50u;
    cv::Mat frame, mask;
    size_t const S = cap.get(cv::CAP_PROP_FRAME_COUNT);
    for (size_t i = 0u; i < S; ++i)
    {
        cap.read(frame);

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        mask = create_mask(frame, background, seuil);
        cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        count = 0u;

        std::vector<cv::Point2f> centers(contours.size());
        std::vector<float> radius(contours.size());
        for (size_t i = 0; i < contours.size(); ++i)
        {
            cv::drawContours(frame, contours, int(i), cv::Scalar(255,0,0), 2, cv::LINE_8, hierarchy, 0);
            cv::minEnclosingCircle(contours[i], centers[i], radius[i]);
        }

        cv::Rect r(cv::Point(70, 315), cv::Point(500, 360));
        cv::rectangle(frame, r, cv::Scalar(255, 0, 0), 5);
        for (size_t i = 0u; i < contours.size(); ++i)
        {
            if ((radius[i] > 20.0f) && (r.contains(centers[i])))
            {
                cv::circle(frame, centers[i], 5, cv::Scalar(0,0,255), 10);//cv::FILLED);
                count++;
            }
        }
        if (count > prev_count)
        {
            cars++;
        }
        prev_count = count;
        cv::rectangle(frame, cv::Point(500, 309), cv::Point(640, 360+35), cv::Scalar(255, 0, 0), cv::FILLED);
        cv::putText(frame, std::to_string(cars), cv::Point(500+10,315+35), cv::FONT_HERSHEY_COMPLEX_SMALL, 2, cv::Scalar(255, 255, 255), 2);

        imshow("Contours", frame);
        //cv::waitKey(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return EXIT_SUCCESS;
}
