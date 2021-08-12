#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// This is a copy of
// https://stackoverflow.com/questions/34959455/detect-parking-lot-by-opencv
// also available on
// https://github.com/stereomatchingkiss/blogCodes2/blob/master/forum_quest/so_obj_detect_00.cpp

static void identify_ob_by_edges(cv::Mat const &img)
{
    cv::Mat gray;
    cv::cvtColor(img, gray, CV_BGR2GRAY);
    cv::threshold(gray, gray, 0, 255,
                  cv::THRESH_BINARY | cv::THRESH_OTSU);
    auto const kernel =
            cv::getStructuringElement(cv::MORPH_RECT, {7,7});
    cv::dilate(gray, gray, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(gray.clone(), contours, cv::RETR_TREE,
                     cv::CHAIN_APPROX_SIMPLE);
    cv::Mat img_copy = img.clone();
    for(auto const &contour : contours){
        auto const rect = cv::boundingRect(contour);
        if(rect.area() >= 2000 &&
                (rect.height / static_cast<double>(rect.width)) > 1.0){
            cv::rectangle(img_copy, rect, {255, 0, 0}, 3);
        }
    }

    cv::imshow("binarize_identify_ob_by_edges", gray);
    cv::imshow("color_identify_ob_by_edges", img_copy);
    cv::imwrite("identify_ob_by_edges.jpg", img_copy);
}

static float line_length(cv::Point2f const &p1,
                  cv::Point2f const &p2)
{
    return std::sqrt(std::pow(p1.x - p2.x,2) +
                     std::pow(p1.y - p2.y,2));
}

static void remove_adjacent_lines(std::vector<cv::Vec4i> &lines, int direction,
                           int threshold = 10)
{
    std::sort(std::begin(lines), std::end(lines),
              [=](cv::Vec4i const &lhs, cv::Vec4i const &rhs)
    {
        return lhs[direction] < rhs[direction];
    });

    for(size_t i = 0; i != lines.size() - 1;){
        if(std::abs(lines[i][0] - lines[i+1][0]) < threshold){
            lines.erase(std::begin(lines) + i);
        }else{
            ++i;
        }
        if(lines.size() < 2){
            break;
        }
    }
}

/**
 * find absolute value of angle between two points
 * @param p1 self explain
 * @param p2 self explain
 * @return angle between two points
 */
static float abs_line_angle(cv::Point2f const &p1,
                     cv::Point2f const &p2)
{
    float const x_diff = std::abs(p2.x- p1.x);
    float const y_diff = std::abs(p2.y- p1.y);

    return static_cast<float>(std::atan(y_diff/x_diff) * 180.0 / CV_PI);
}

/**
 * Extend the line(based on unit vector)
 * @param increase_ratio increase the lines how many times
 * @param length length of the original line
 * @param p1 self explain
 * @param p2 self explain
 * @return new point for line extend
 */
static cv::Point2f extend_line(float increase_ratio,
                        float length,
                        cv::Point2f const &p1,
                        cv::Point2f const &p2)
{
    cv::Point2f new_point;
    new_point.x = p1.x + (p2.x - p1.x) / length
            * increase_ratio;
    new_point.y = p1.y + (p2.y - p1.y) / length
            * increase_ratio;

    return new_point;

}

/**
 * Extend the line on two sides and draw it on img
 * @param img image for drawing line
 * @param increase_ratio increase the lines how many times
 * @param length length of the original line
 * @param p1 self explain
 * @param p2 self explain
 * @param color color of the line
 * @param thickness thickness of line you want to draw
 * @return new point for line extend
 */
static void extend_line(cv::Mat &img,
                 float increase_ratio,
                 cv::Point2f const &p1,
                 cv::Point2f const &p2,
                 cv::Scalar const &color,
                 int thickness = 1)
{
    float const length = line_length(p1, p2);
    auto new_point = extend_line(increase_ratio, length,
                                 p1, p2);
    cv::line(img, p1, new_point, color, 2);
    new_point = extend_line(increase_ratio, length,
                            p2, p1);
    cv::line(img, p2, new_point, color, thickness);
}

static void draw_lines(cv::Mat &img, std::vector<cv::Vec4i> const &lines,
                cv::Scalar const &color, int thickness = 1)
{
    for(auto const &l : lines)
    {
        auto const p1 = cv::Point(l[0], l[1]);
        auto const p2 = cv::Point(l[2], l[3]);
        extend_line(img, 1000, p1, p2,
                    color, thickness);
    }
}

static void identify_ob_by_lines(cv::Mat const &img)
{
    cv::Mat gray;
    cv::cvtColor(img, gray, CV_BGR2GRAY);
    cv::threshold(gray, gray, 0, 255,
                  cv::THRESH_BINARY | cv::THRESH_OTSU);

    cv::Mat edges;
    cv::Canny(gray, edges, 30, 90);
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1,
                    CV_PI/180, 50, 50, 10);

    std::vector<cv::Vec4i> hor_lines;
    std::vector<cv::Vec4i> vec_lines;
    //remove lines with invalid angle
    for(auto const &l : lines)
    {
        auto const p1 = cv::Point(l[0], l[1]);
        auto const p2 = cv::Point(l[2], l[3]);
        auto const angle = abs_line_angle(p1, p2);
        if(angle >= 76){
            vec_lines.emplace_back(l);
        }else if(angle <= 5){
            hor_lines.emplace_back(l);
        }
    }

    //remove_adjacent_lines(hor_lines, 1, 400);
    remove_adjacent_lines(vec_lines, 0, 30);

    //draw lines on blank image
    cv::Mat blank = cv::Mat::zeros(img.size(), CV_8U);
    draw_lines(blank, hor_lines, {255});
    draw_lines(blank, vec_lines, {255});

    //find the contours of blank image
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(blank.clone(), contours, cv::RETR_TREE,
                     cv::CHAIN_APPROX_SIMPLE);
    for(auto const &contour : contours){
        auto const rect = cv::boundingRect(contour);
        if(rect.area() >= 2000 &&
                (rect.height / static_cast<double>(rect.width)) > 1.0){
            //cv::rectangle(img_copy, rect, {255, 0, 0}, 3);
            auto const min_rect = cv::minAreaRect(contour);
            cv::Point2f rect_points[4];
            min_rect.points(rect_points);
            for(size_t j = 0; j < 4; ++j){
                cv::line(img, rect_points[j],
                         rect_points[(j+1)%4], {255, 0, 0}, 2, 8);
            }
        }
    }

    cv::imshow("img copy_identify_ob_by_lines", img);
    cv::imwrite("identify_ob_by_lines.jpg", blank);
}

//******************************************************************************
// g++ -W -Wall --std=c++11 ParkingLots.cpp -o ParkingLots `pkg-config --libs --cflags opencv`
//******************************************************************************
int main()
{
    // Algorithm 1
    identify_ob_by_edges(cv::imread("parking.jpg"));

    // Alternative algorithm
    identify_ob_by_lines(cv::imread("parking.jpg"));

    cv::waitKey();

    return 0;
}
