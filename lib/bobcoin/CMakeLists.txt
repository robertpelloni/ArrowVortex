cmake_minimum_required(VERSION 3.28)
project(Bobcoin)

add_library(bobcoin STATIC src/bobcoin.cpp src/ProofOfDance.cpp)
target_include_directories(bobcoin PUBLIC include src)
