cmake_minimum_required(VERSION 3.10)
project(CivilRegistryAnalyser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(OpenCV REQUIRED)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(thirdparty/json)

add_subdirectory(thirdparty/pybind11)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

execute_process(COMMAND "conda activate HandWrittenTextRecognitionTutorial")

set(HEADERS
        "include/DatasetBuilder/DatasetBuilder.hpp"
        "include/DatasetBuilder/DatasetImage.hpp"
        "include/DatasetBuilder/ImageSet.hpp"
        "include/DatasetBuilder/ImageSegmenterDialog.hpp"
        "include/ImageUtil.hpp"
        "include/TextSegmentation/Binarization.hpp"
        "include/TextSegmentation/LineSegmentation.hpp"
        "include/TextSegmentation/Scanner.hpp"
        "include/TextSegmentation/WordSegmentation.hpp"
        "include/TextSegmentation/TextSegmentation.hpp"
        )

set(SOURCES
        "src/main.cpp"
        "src/DatasetBuilder/DatasetBuilder.cpp"
        "src/DatasetBuilder/DatasetImage.cpp"
        "src/DatasetBuilder/ImageSet.cpp"
        "src/DatasetBuilder/ImageSegmenterDialog.cpp"
        "src/TextSegmentation/Binarization.cpp"
        "src/TextSegmentation/LineSegmentation.cpp"
        "src/TextSegmentation/Scanner.cpp"
        "src/TextSegmentation/WordSegmentation.cpp"
        "src/TextSegmentation/TextSegmentation.cpp")

set(UI
        "src/DatasetBuilder/DatasetBuilder.ui"
        "src/DatasetBuilder/ImageSegmenterDialog.ui"
    )

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UI})
add_executable(tftest "src/TfMain.cpp")

target_compile_options(${PROJECT_NAME} INTERFACE)
if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Wwrite-strings -std=c++17 -Wno-psabi -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-deprecated -lstdc++fs)
elseif (MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17 /Zc:__cplusplus /MDd /Od /EHsc /wd4996 /Zi /wd4634 /wd4635)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17 /Zc:__cplusplus /DNDEBUG /O2 /Ob2 /EHsc /Zi /wd4996 /wd4634 /wd4635)
    endif()
endif ()

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(tftest SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(tftest PRIVATE pybind11::embed)

target_link_libraries(tftest PRIVATE ${OpenCV_LIBS})

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Widgets
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json)
