cmake_minimum_required(VERSION 3.26)
project(SOCKS5_proxy)

cmake_policy(SET CMP0144 NEW)
add_definitions(-D_WIN32_WINNT=0x0601)

set(CMAKE_CXX_STANDARD 23)


add_executable(SOCKS5_proxy
        main.cpp
        Colors.hpp
        Main_server.hpp
        Main_server.cpp)

set(BOOST_ROOT "C:/Program Files/Boost/boost_1_83_0")
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(SOCKS5_proxy ${Boost_LIBRARIES})
