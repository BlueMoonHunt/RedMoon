cmake_minimum_required(VERSION 3.10)

project(RedMoon VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(RedMoon redmoon.cpp ${SOURCES})
target_include_directories(RedMoon PRIVATE src)
target_compile_features(RedMoon PRIVATE cxx_std_23)
