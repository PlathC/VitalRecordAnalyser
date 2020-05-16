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

#execute_process(COMMAND "conda activate HandWrittenTextRecognitionTutorial")

set(HEADERS
        "include/Ui/DatasetBuilder.hpp"
        "include/Ui/ImageSegmenterDialog.hpp"
        "include/Ui/ImageUtil.hpp"
        "include/Model/Dataset/DatasetImage.hpp"
        "include/Model/Dataset/ImageSet.hpp"
        "include/Model/TextSegmentation/Binarization.hpp"
        "include/Model/TextSegmentation/LineSegmentation.hpp"
        "include/Model/TextSegmentation/Scanner.hpp"
        "include/Model/TextSegmentation/WordSegmentation.hpp"
        "include/Model/TextSegmentation/TextSegmentation.hpp"
        "include/Model/TextDetection/TextDetection.hpp"
        include/Model/TextDetection/MatCaster.hpp)

set(SOURCES
        "src/DatasetBuilder/main.cpp"
        "src/Ui/DatasetBuilder.cpp"
        "src/Ui/ImageSegmenterDialog.cpp"
        "src/Model/Dataset/DatasetImage.cpp"
        "src/Model/Dataset/ImageSet.cpp"
        "src/Model/TextSegmentation/Binarization.cpp"
        "src/Model/TextSegmentation/LineSegmentation.cpp"
        "src/Model/TextSegmentation/Scanner.cpp"
        "src/Model/TextSegmentation/WordSegmentation.cpp"
        "src/Model/TextSegmentation/TextSegmentation.cpp"
        "src/Model/TextDetection/TextDetection.cpp"
        )

set(UI
        "src/Ui/DatasetBuilder.ui"
        "src/Ui/ImageSegmenterDialog.ui"
    )

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UI})

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

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Widgets
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
        pybind11::embed)

file(GLOB PYTHONFILES
        "py/*"
        )
file(COPY ${PYTHONFILES} DESTINATION py)
