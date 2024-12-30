project(
        sha256
        VERSION 1.0.0
        DESCRIPTION
        "A simple C++ project to demonstrate creating executables and libraries in CMake"
        LANGUAGES CXX
)

add_library(sha256 sha256.cpp)