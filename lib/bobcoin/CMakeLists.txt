cmake_minimum_required(VERSION 3.28)
project(Bobcoin)

add_library(bobcoin STATIC src/bobcoin.cpp)
target_include_directories(bobcoin PUBLIC include)
