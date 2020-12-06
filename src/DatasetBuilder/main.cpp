#include <chrono>
#include <iostream>

#include <QApplication>
//
// #include "Ui/DatasetBuilder.hpp"

#ifdef slots
#undef slots
#endif

#include "Model/TextSegmentation/EASTDetector.hpp"
#include "Model/TextSegmentation/Preprocessing.hpp"
#include "Model/TextSegmentation/Segmentation.hpp"

#define slots

int main(int argc, char** argv) {

    auto img = cv::imread("6kSample.jpg", cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    img = preprocessing::ExtractBiggestFeature(img);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Border's removing = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.
              << "[s]" << std::endl;

    begin = std::chrono::steady_clock::now();
    img = preprocessing::GammaCorrection(img);
    end = std::chrono::steady_clock::now();
    std::cout << "Gamma Correction = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.
              << "[s]" << std::endl;

    begin = std::chrono::steady_clock::now();
    img = preprocessing::LocallySoftAdaptiveBinarization(img);
    end = std::chrono::steady_clock::now();
    std::cout << "Locally Soft Adaptative Binarization = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.
              << "[s]" << std::endl;

    begin = std::chrono::steady_clock::now();
    auto quarters = segmentation::SegmentCivilStates(img);
    end = std::chrono::steady_clock::now();
    std::cout << "Quarters = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.
              << "[s]" << std::endl;

    cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);

    namespace py = pybind11;
    py::scoped_interpreter interpreter{};
    segmentation::EASTDetector detector{};
    detector.Process(img);

    return EXIT_SUCCESS;

    // cv::imwrite("test.png", img);
    //QApplication app{argc, argv};
    // DatasetBuilder::DatasetBuilder w indow;
    // window.show();

    //return app.exec();
}