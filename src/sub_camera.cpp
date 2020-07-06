#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <cstdio>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <zmq.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv) {
  bool verbose = false;

  po::options_description desc("Options");
  desc.add_options()("help,h", "print help message");
  desc.add_options()("verbose,v", po::value<bool>(&verbose)->default_value(verbose), "Shows camera view");

  po::variables_map variables_map;
  po::store(po::parse_command_line(argc, argv, desc), variables_map);
  po::notify(variables_map);

  if (variables_map.count("help")) {
    spdlog::info("No arguments were provided");
    std::cout << desc << std::endl;
  }

  spdlog::info("Openning zmq context");
  zmq::context_t context(1);
  zmq::socket_t subscriber(context, ZMQ_SUB);
  // ToDo: set this value from the console arguments
  subscriber.connect("tcp://192.168.0.11:5555");

  subscriber.setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
  if (verbose) {
    cv::namedWindow("output", CV_WINDOW_AUTOSIZE);
  }

  do {
    double start_ticks = static_cast<double>(cv::getTickCount());
    zmq::message_t update;

    subscriber.recv(&update);
    cv::Mat img;
    char *ptr_data = reinterpret_cast<char *>(update.data());
    std::vector<uchar> data(ptr_data, ptr_data + update.size());
    spdlog::info("Data received: {0:d}", data.size());
    cv::imdecode(data, cv::ImreadModes::IMREAD_COLOR, &img);
    if (verbose) {
      cv::imshow("output", img);
    }

    double end_ticks = static_cast<double>(cv::getTickCount());
    double elapsed_time = (end_ticks - start_ticks) / cv::getTickFrequency();
    spdlog::info("Frame processing time: {:03.6f}", elapsed_time);
  } while (cv::waitKey(1) != 'q');

  subscriber.close();
  cv::destroyAllWindows();
}

