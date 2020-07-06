project (MichiSurveillance)
cmake_minimum_required(VERSION 3.12)

## 
set(CMAKE_CXX_STANDARD 14)

## use pkg-config to get hints for 0mq locations
# pkg_check_modules(PKG_ZeroMQ QUIET zmq)

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

## pub_camera
add_executable (pub_camera src/pub_camera.cpp)
target_link_libraries(pub_camera ${ZeroMQ_LIBS} ${OpenCV_LIBS} ${Boost_LIBRARIES} spdlog::spdlog)

## pub_camera
add_executable (sub_camera src/sub_camera.cpp)
target_link_libraries(sub_camera ${ZeroMQ_LIBS} ${OpenCV_LIBS} ${Boost_LIBRARIES} spdlog::spdlog)
