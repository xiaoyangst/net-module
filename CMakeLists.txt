cmake_minimum_required(VERSION 3.16)
project(net)

set(CMAKE_CXX_STANDARD 17)

#找到头文件
include_directories(${PROJECT_SOURCE_DIR}/third-party)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)
