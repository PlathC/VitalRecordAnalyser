#ifndef CIVILREGISTRYANALYSER_LOCALLYSOFTADAPTATIVEBINARIZATION_HPP
#define CIVILREGISTRYANALYSER_LOCALLYSOFTADAPTATIVEBINARIZATION_HPP

#include <opencv2/opencv.hpp>

namespace preprocessing
{
    cv::Mat LocallySoftAdaptiveBinarization(const cv::Mat& img, uint16_t blockSize=200);

    cv::Mat ApplyAdaptiveMedianThreshold(const cv::Mat& inout, uint8_t delta = 30);

    cv::Mat CombineAdaptativeBlock(const cv::Mat& origin, const cv::Mat& mask);
}

#endif //CIVILREGISTRYANALYSER_LOCALLYSOFTADAPTATIVEBINARIZATION_HPP
