cmake_minimum_required(VERSION 3.30)
project(pageant_keyLoader)
set(CMAKE_CXX_STANDARD 23)
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL/")

find_package(OpenSSL REQUIRED)

set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp)

add_executable(pageant_keyLoader main.cpp)

target_link_libraries(pageant_keyLoader
        SQLiteCpp
        sqlite3
        pthread
)
target_link_libraries(pageant_keyLoader OpenSSL::Crypto)
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_link_libraries(pageant_keyLoader -static)
endif()

add_executable(test test.cpp)
target_link_libraries(test
        SQLiteCpp
        sqlite3
        pthread
)
target_link_libraries(test OpenSSL:Crypto)