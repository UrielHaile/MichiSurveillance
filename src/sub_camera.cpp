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
#include <yolo_v2_class.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv) {
  bool verbose = false;
  std::string cfg_filename;
  std::string weights_filename;

  po::options_description desc("Options");
  desc.add_options()("help,h", "print help message");
  desc.add_options()("verbose,v", po::value<bool>(&verbose)->default_value(verbose), "Shows camera view");
  desc.add_options()("weights,w", po::value<std::string>(&weights_filename), "Weights file");
  desc.add_options()("cfg,c", po::value<std::string>(&cfg_filename), "Configuration file");

  po::variables_map variables_map;
  po::store(po::parse_command_line(argc, argv, desc), variables_map);
  po::notify(variables_map);

  if (variables_map.count("help")) {
    spdlog::info("No arguments were provided");
    std::cout << desc << std::endl;
    exit(0);
  } else 

  if (variables_map.count("weights")) {
    spdlog::info("Weights file: {0}", weights_filename);
  } else {
    spdlog::error("No weights file was provided"); 
    spdlog::info("please use '-w [ --weights ]'"); 
    exit(0);
  }

  if (variables_map.count("cfg")) {
    spdlog::info("Cfg file: {0}", cfg_filename);
  } else {
    spdlog::error("No configuration file was provided"); 
    spdlog::info("please use '-c [ --cfg ]'"); 
    exit(0);
  }

  Detector detector(cfg_filename, weights_filename, 0);
  spdlog::info("Openning zmq context");
  zmq::context_t context(1);
  zmq::socket_t subscriber(context, ZMQ_SUB);
  // ToDo: set this value from the console arguments
  subscriber.connect("tcp://192.168.0.11:5555");

  subscriber.setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
  if (verbose) {
    cv::namedWindow("output", cv::WINDOW_AUTOSIZE);
  }
  
  preview_boxes_t preview;

  do {
    double start_ticks = static_cast<double>(cv::getTickCount());
    zmq::message_t update;

    subscriber.recv(&update);
    cv::Mat img;
    char *ptr_data = reinterpret_cast<char *>(update.data());
    std::vector<uchar> data(ptr_data, ptr_data + update.size());
    spdlog::info("Data received: {0:d}", data.size());
    cv::imdecode(data, cv::ImreadModes::IMREAD_COLOR, &img);
    std::vector<bbox_t> detections = detector.detect(img);
    if (verbose) {
      preview.set(img, detections);
      preview.draw(img);
      cv::imshow("output", img);
    }

    double end_ticks = static_cast<double>(cv::getTickCount());
    double elapsed_time = (end_ticks - start_ticks) / cv::getTickFrequency();
    spdlog::info("Frame processing time: {:03.6f}", elapsed_time);
  } while (cv::waitKey(1) != 'q');

  subscriber.close();
  cv::destroyAllWindows();
}

