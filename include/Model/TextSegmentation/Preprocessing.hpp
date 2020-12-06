#ifndef CIVILREGISTRYANALYSER_PREPROCESSING_HPP
#define CIVILREGISTRYANALYSER_PREPROCESSING_HPP

#include <string>

#include <opencv2/opencv.hpp>

#include "Model/TextSegmentation/LocallySoftAdaptativeBinarization.hpp"

namespace preprocessing
{
    cv::Mat GammaCorrection(const cv::Mat& img, float gamma=2);
    cv::Mat ExtractBiggestFeature(const cv::Mat& img, uint16_t horizontalDilation=100);
}

#endif //CIVILREGISTRYANALYSER_PREPROCESSING_HPP
