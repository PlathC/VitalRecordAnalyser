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

message("VAL ==> $ENV{TENSORFLOW_BUILD_DIR}")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

#set(TF_INCLUDE
#        "D:\\Library\\tensorflow\\bazel-tensorflow"
#        "D:\\Library\\tensorflow\\bazel-tensorflow\\external\\com_google_absl"
#        "D:\\Library\\tensorflow\\bazel-tensorflow\\external\\eigen_archive"
#        "D:\\Library\\tensorflow\\bazel-tensorflow\\external\\com_google_protobuf\\src"
#        "D:\\Library\\tensorflow\\bazel-bin"
#        )
#set(TF_LIBRARY "D:\\Library\\tensorflow\\bazel-bin\\tensorflow\\tensorflow.lib")

find_package(Tensorflow REQUIRED)

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

    if(MARIONETTE_DYNAMIC)
            target_compile_definitions(${PROJECT_NAME} PRIVATE MARIONETTE_DYNAMIC)
    endif()
endif ()

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(tftest SYSTEM PRIVATE ${TensorFlow_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Widgets
        ${OpenCV_LIBS} )

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(tftest ${TensorFlow_LIBRARY})