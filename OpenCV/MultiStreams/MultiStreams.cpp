#include <opencv2/opencv.hpp>
#include <zmq.hpp>
#include <apriltag/apriltag.h>
#include <apriltag/tag25h9.h>

#include <functional>
#include <future>
#include <vector>
#include <iostream>

// *****************************************************************************
//! \brief Stream a video device (i.e. /dev/video0) inside a thread and do an
//! image processing on the captured frame.
// *****************************************************************************
class StreamProcessor
{
public:

    // -------------------------------------------------------------------------
    //! \brief Default constructor opening the given video device
    //! (i.e. /dev/video0).
    // -------------------------------------------------------------------------
    StreamProcessor(std::string const& device)
        : m_device(device)
    {
        m_capture.open(device);
        if (!m_capture.isOpened())
        {
            std::cerr << "ERROR: Unable to open the camera device '"
                      << device << "'" << std::endl;
            return ;
        }
    }

    // -------------------------------------------------------------------------
    ~StreamProcessor() { stop(); }

    // -------------------------------------------------------------------------
    std::string const& device() const { return m_device; }

    // -------------------------------------------------------------------------
    //! \brief Return the last captured frame.
    // -------------------------------------------------------------------------
    cv::Mat frame()
    {
        std::lock_guard<std::mutex> lock(m_frame_mutex);
        m_frame_available = false;
        return m_frame;
    }

    // -------------------------------------------------------------------------
    //! \brief Start the thread given the image processing and the context.
    //! \tparam Context a context to be used during the image processing.
    //! \tparam Function a std::function<bool(cv::Mat&, Context&)> doing the
    //! image processing on the latest captured frame.
    //! \return true if the thread has started else false in case of error
    //! (thread already open or video device not opened).
    // -------------------------------------------------------------------------
    template <typename Context, typename Function>
    bool start(Context& context, Function const& process_frame)
    {
        if (m_is_running)
            return true;

        if (!m_capture.isOpened())
            return false;

        m_is_running = true;
        m_processing_thread =
                std::thread(&StreamProcessor::process<Context, Function>,
                            this, std::ref(context), std::ref(process_frame));
        return true;
    }

    // -------------------------------------------------------------------------
    //! \brief Stop the image processing thread.
    // -------------------------------------------------------------------------
    void stop()
    {
        m_is_running = false;
        if (m_processing_thread.joinable())
            m_processing_thread.join();
    }

private:

    // -------------------------------------------------------------------------
    //! \brief The image processing thread: read a video frame and call the
    //! image processing on it.
    // -------------------------------------------------------------------------
    template <typename Context, typename Function>
    void process(Context& context, Function const& process_frame)
    {
        while (m_is_running)
        {
            cv::Mat frame;
            if (!m_capture.read(frame))
            {
                std::cerr << "ERROR: Unable to get the frame "
                          << "for the camera device '"
                          << m_device << "'" << std::endl;
                continue;
            }

            if (frame.empty())
                continue;

            if (process_frame(frame, context))
            {
                std::lock_guard<std::mutex> lock(m_frame_mutex);
                m_frame = frame.clone();
                m_frame_available = true;
            }
        }
    }

private:

    std::string m_device;
    cv::VideoCapture m_capture;
    cv::Mat m_frame;
    std::thread m_processing_thread;
    std::mutex m_frame_mutex;
    bool m_is_running = false;
    bool m_frame_available = false;
};

// *****************************************************************************
//! \brief Detect an April tag (tag25h9 familly).
//! https://github.com/AprilRobotics/apriltag
// *****************************************************************************
class AprilTagDetection
{
public:

    // -------------------------------------------------------------------------
    //! \brief
    // -------------------------------------------------------------------------
    AprilTagDetection()
    {
        m_familly = tag25h9_create();
        m_detector = apriltag_detector_create();
        apriltag_detector_add_family(m_detector, m_familly);

        m_detector->quad_decimate = 2.0;
        m_detector->quad_sigma = 0.0;
        m_detector->nthreads = 1;
        m_detector->debug = 0;
        m_detector->refine_edges = 1;
    }

    // -------------------------------------------------------------------------
    //! \brief
    // -------------------------------------------------------------------------
    ~AprilTagDetection()
    {
        apriltag_detector_destroy(m_detector);
        tag25h9_destroy(m_familly);
    }

    // -------------------------------------------------------------------------
    //! \brief Detect an April tag tag25h9 if present.
    //! \return true if detected.
    // -------------------------------------------------------------------------
    bool detect(cv::Mat& frame) const
    {
        image_u8_t image = {
            .width = frame.cols,
            .height = frame.rows,
            .stride = frame.cols,
            .buf = frame.data
        };

        zarray_t* detections = apriltag_detector_detect(m_detector, &image);
        const auto count = zarray_size(detections);
        for (int i = 0; i < count; i++)
        {
            apriltag_detection_t* det;
            zarray_get(detections, i, &det);
            cv::line(frame, cv::Point(det->p[0][0], det->p[0][1]),
                     cv::Point(det->p[1][0], det->p[1][1]),
                     cv::Scalar(0, 0xff, 0), 2);
            cv::line(frame, cv::Point(det->p[0][0], det->p[0][1]),
                     cv::Point(det->p[3][0], det->p[3][1]),
                     cv::Scalar(0, 0, 0xff), 2);
            cv::line(frame, cv::Point(det->p[1][0], det->p[1][1]),
                     cv::Point(det->p[2][0], det->p[2][1]),
                     cv::Scalar(0xff, 0, 0), 2);
            cv::line(frame, cv::Point(det->p[2][0], det->p[2][1]),
                     cv::Point(det->p[3][0], det->p[3][1]),
                     cv::Scalar(0xff, 0, 0), 2);
        }

        apriltag_detections_destroy(detections);
        return count != 0;
    }

private:

    apriltag_detector_t* m_detector;
    apriltag_family_t* m_familly;
};

// *****************************************************************************
//! \brief Application class. Has an April detector and publish a 0MQ message if
//! detected.
// *****************************************************************************
class ZMQ
{
public:

    // -------------------------------------------------------------------------
    //! \brief Default constructor with a 0MQ address.
    // -------------------------------------------------------------------------
    ZMQ(std::string const& addr)
        : m_zmq_context(1), m_zmq_socket(m_zmq_context, ZMQ_PUB)
    {
        m_zmq_socket.bind(addr);
    }

    // -------------------------------------------------------------------------
    //! \brief Publish a 0MQ message.
    // -------------------------------------------------------------------------
    void publish(std::string const& payload)
    {
        std::cout << "Publishing: " << payload << std::endl;
        zmq::message_t message(payload.size());
        memcpy(message.data(), payload.c_str(), payload.size());
        m_zmq_socket.send(message, zmq::send_flags::none);
    }

private:

    zmq::context_t m_zmq_context;
    zmq::socket_t m_zmq_socket;
};

// *****************************************************************************
//! \brief
// *****************************************************************************
class AprilTagDetectorWorker
{
public:

    Worker(ZMQ& zmq, std::string const& device)
        : m_zmq(zmq), processor(device)
    {
        processor.stop();
    }

    bool start()
    {
        // Image processing detecting an April tag.
        bool started = processor.start(*this, [](cv::Mat& frame, Worker& worker)
        {
            cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);
            if (worker.detector.detect(frame))
            {
                std::string payload("Tag detected by camera ");
                worker.m_zmq.publish(payload + worker.processor.device());
            }
            return true;
        });

        return started;
    }

private:

    ZMQ& m_zmq;

public:

    StreamProcessor processor;
    TagDetector detector;
};

// *****************************************************************************
// g++ -Wall -Wextra -Wshadow MultiStreams.cpp `pkg-config --cflags opencv4
// libzmq apriltag` -o video_processor `pkg-config --libs opencv4 libzmq apriltag`
// -lpthread
// *****************************************************************************
int main(int argc, char* argv[])
{
    ZMQ zmq("tcp://*:5555");
    Worker w1(zmq, "/dev/video0");
    Worker w2(zmq, "/dev/video2");

    if (!w1.start())
    {
        std::cerr << "Failed to start worker 1" << std::endl;
        return EXIT_FAILURE;
    }

    if (!w2.start())
    {
        std::cerr << "Failed to start worker 2" << std::endl;
        return EXIT_FAILURE;
    }

    while (true)
    {
        cv::Mat frame = w1.processor.frame();
        if (!frame.empty())
            cv::imshow(w1.processor.device(), frame);

        frame = w2.processor.frame();
        if (!frame.empty())
            cv::imshow(w2.processor.device(), frame);

        int key = cv::waitKey(1);
        if (key == 27)
            break;
    }

    return EXIT_SUCCESS;
}
