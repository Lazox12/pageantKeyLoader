cmake_minimum_required(VERSION 3.30)
project(pageant_keyLoader)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 99)
set(ENABLE_NVIDIA OFF)
set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)
set(PUTTY_LIB_DIR ${CMAKE_SOURCE_DIR}/putty)


find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/repo/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()


add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/sha256)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/base64)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/hmac_sha256)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/argon2-cpp)
add_executable(pageant_keyLoader main.cpp
        sshKey.cpp)

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
target_include_directories(pageant_keyLoader PUBLIC ${CMAKE_CURRENT_LIST_DIR}/thirdparty/argon2-cpp)
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_link_libraries(pageant_keyLoader -static)
endif()

