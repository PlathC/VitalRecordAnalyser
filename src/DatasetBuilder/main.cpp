// #include <QApplication>
//
// #include "Ui/DatasetBuilder.hpp"


#include "Model/TextSegmentation/Preprocessing.hpp"

int main(int argc, char** argv) {
    auto img = cv::imread("6kSample.jpg", cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR);
    img = preprocessing::ExtractBiggestFeature(img);
    img = preprocessing::GammaCorrection(img);
    img = preprocessing::LocallySoftAdaptativeBinarization(img);

    cv::imwrite("test.png", img);
    // QApplication app{argc, argv};
    // DatasetBuilder::DatasetBuilder window;
    // window.show();
//
    // return app.exec();
}