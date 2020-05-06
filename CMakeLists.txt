cmake_minimum_required(VERSION 3.10)
project(CivilRegistryAnalyser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(OpenCV REQUIRED)

set(HEADERS
        "include/DatasetBuilder/MainWindow.hpp"
        "include/DatasetBuilder/DatasetImage.hpp"
        include/DatasetBuilder/ImageSet.hpp)

set(SOURCES
        "src/main.cpp"
        "src/DatasetBuilder/MainWindow.cpp"
        src/DatasetBuilder/DatasetImage.cpp
        src/DatasetBuilder/ImageSet.cpp
        )
set(UI
        "src/DatasetBuilder/MainWindow.ui")

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UI})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME}
        Qt5::Widgets
        ${OpenCV_LIBS} )
