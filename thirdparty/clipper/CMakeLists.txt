# CMake configuration file for clipper integration
# Original website : http://www.angusj.com/delphi/clipper.php

cmake_minimum_required (VERSION 3.3)

project("clipper")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

list(APPEND HEADER_FILE
        "include/clipper/clipper.hpp")

list(APPEND SOURCE_FILE
        "src/clipper/clipper.cpp")

add_library(${PROJECT_NAME} STATIC ${HEADER_FILE} ${SOURCE_FILE})
message("Create clipper Static library")

target_compile_options(${PROJECT_NAME} INTERFACE)
if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Wwrite-strings -std=c++17 -Wno-psabi -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-deprecated -lstdc++fs)
elseif (MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17 /Zc:__cplusplus /MDd /Od /EHsc /wd4996 /Zi /wd4634 /wd4635)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17 /Zc:__cplusplus /DNDEBUG /O2
                # /Ob2
                /EHsc /Zi /wd4996 /wd4634 /wd4635)
        target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:CONSOLE# /ENTRY:mainCRTStartup
                )
    endif()
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE include/
        SYSTEM PUBLIC include/)

target_include_directories(${PROJECT_NAME} INTERFACE include/)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)