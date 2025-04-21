cmake_minimum_required(VERSION 3.20)

project(alx-home_cpp_utils LANGUAGES CXX)

win32_library(TARGET_NAME alx-home_cpp_utils
  FILES
    src/String.cpp
)
add_library(alx-home::cpp_utils ALIAS alx-home_cpp_utils)

target_include_directories(alx-home_cpp_utils PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
