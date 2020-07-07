#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <cstdio>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <zmq.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv) {
  int camera_index = 0;
  bool verbose = false;

  po::options_description desc("Options");
  desc.add_options()("help,h", "print help message");
  desc.add_options()("camera_index,c", po::value<int>(&camera_index)->default_value(camera_index),
                     "Sets the camera index - default is false");
  desc.add_options()("verbose,v", po::value<bool>(&verbose)->default_value(verbose), "Shows camera view");

  po::variables_map variables_map;
  po::store(po::parse_command_line(argc, argv, desc), variables_map);
  po::notify(variables_map);

  if (variables_map.count("help")) {
    spdlog::info("No arguments were provided");
    std::cout << desc << std::endl;
  }

  zmq::context_t context(1);
  zmq::socket_t publisher(context, ZMQ_PUB);
  // ToDo: set this value from the console arguments
  publisher.bind("tcp://192.168.0.11:5555");

  cv::VideoCapture capture(camera_index);
  if (!capture.isOpened()) {
    spdlog::error("Unable to open video source, terminating program!");
    return 0;
  }

  spdlog::info("The camera was succefully open!");

  if (verbose) {
    cv::namedWindow("output", cv::WINDOW_AUTOSIZE);
  }

  do {
    double start_ticks = static_cast<double>(cv::getTickCount());
    cv::Mat frame;
    bool capture_success = capture.read(frame);
    zmq::message_t message;

    if (verbose) {
      cv::imshow("output", frame);
    }

    if (capture_success) {
      std::vector<uchar> buffer;
      cv::imencode(".png", frame, buffer);
      spdlog::info("Bytes sent: {0:d}", buffer.size());
      publisher.send((void *)buffer.data(), buffer.size());
    }
    double end_ticks = static_cast<double>(cv::getTickCount());
    double elapsed_time = (end_ticks - start_ticks) / cv::getTickFrequency();
    spdlog::info("Frame processing time: {:03.6f}", elapsed_time);
  } while (cv::waitKey(1) != 'q');

  publisher.close();
  cv::destroyAllWindows();
}
