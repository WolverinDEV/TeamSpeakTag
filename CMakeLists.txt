cmake_minimum_required(VERSION 3.9)
project(TS3DataParser)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
add_executable(TS3DataParser main.cpp utils.h TSTag.hpp)
target_link_libraries(TS3DataParser jsoncpp)