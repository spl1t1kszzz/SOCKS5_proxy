cmake_minimum_required(VERSION 3.26)
project(SOCKS5_proxy)

set(CMAKE_CXX_STANDARD 20)

add_executable(SOCKS5_proxy
        main.cpp
        Colors.hpp
        Main_server.hpp
        Main_server.cpp)

set(BOOST_ROOT "C:/Program Files/Boost/boost_1_83_0")
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(SOCKS5_proxy ${Boost_LIBRARIES})