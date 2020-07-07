project (MichiSurveillance)
cmake_minimum_required(VERSION 3.12)

## 
set(CMAKE_CXX_STANDARD 14)
## Compilation database vim youcompleteme
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

find_path(ZeroMQ_ROOT_DIR NAMES include/zmq.h)

find_library(ZeroMQ_LIBS NAMES zmq libzmq HINTS ${ZeroMQ_ROOT_DIR}/lib)
find_path(ZeroMQ_INCLUDE_DIR NAMES zmq.h HINTS ${ZeroMQ_ROOT_DIR}/include)

# set the include and link directories
include_directories(${ZeroMQ_INCLUDE_DIR})
link_directories(${ZeroMQ_LIB_DIR})

## OpenCV library 
find_package (OpenCV REQUIRED)

## SPDLOG
find_package(spdlog REQUIRED)

## Boost
find_package(Boost REQUIRED COMPONENTS program_options)

## Darknet
find_package(Darknet REQUIRED)

## pub_camera
add_executable (pub_camera src/pub_camera.cpp)
target_link_libraries(pub_camera ${ZeroMQ_LIBS} ${OpenCV_LIBS} ${Boost_LIBRARIES} spdlog::spdlog)

if (Darknet_FOUND)
  include_directories(${Darknet_INCLUDE_DIR})
  ## enable OpenCV
  add_compile_definitions(OPENCV)
  add_executable (sub_camera src/sub_camera.cpp)
  target_link_libraries(sub_camera ${ZeroMQ_LIBS} ${OpenCV_LIBS} ${Boost_LIBRARIES} spdlog::spdlog Darknet::dark)
endif (Darknet_FOUND)
