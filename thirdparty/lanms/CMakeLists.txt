# CMake configuration file for lanms integration
# Original website : https://github.com/argman/EAST/tree/master/lanms

cmake_minimum_required (VERSION 3.3)

set(CMAKE_CXX_STANDARD 17)

project("lanms")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

list(APPEND HEADER_FILE
        "include/lanms/lanms.h")

add_library(${PROJECT_NAME} INTERFACE)

message("Creating lanms library")

target_include_directories(${PROJECT_NAME} INTERFACE include/)
