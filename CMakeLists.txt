cmake_minimum_required(VERSION 3.30)
project(pageant_keyLoader)
set(CMAKE_CXX_STANDARD 23)

set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/sha256)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/base64)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/hmac_sha256)
add_executable(pageant_keyLoader main.cpp)

target_link_libraries(pageant_keyLoader
        SQLiteCpp
        sqlite3
        pthread
        sha256
        hmac_sha256
        base64
)
target_include_directories(pageant_keyLoader PUBLIC ${CMAKE_CURRENT_LIST_DIR}/thirdparty/sha256)
target_include_directories(pageant_keyLoader PUBLIC ${CMAKE_CURRENT_LIST_DIR}/thirdparty/base64)
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_link_libraries(pageant_keyLoader -static)
endif()

