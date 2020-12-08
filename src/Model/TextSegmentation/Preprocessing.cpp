#include "Model/TextSegmentation/Preprocessing.hpp"

namespace preprocessing
{
    cv::Mat GammaCorrection(const cv::Mat& img, float gamma)
    {
        float invGamma = 1.f / gamma;
        std::array<float, 256> lookUpTable{};
        for(size_t i = 0; i < lookUpTable.size(); i++)
        {
            lookUpTable[i] = (std::pow((static_cast<float>(i) / 255.f), invGamma)) * 255;
        }
        cv::Mat out = cv::Mat(img.rows, img.cols, CV_8UC1);
        cv::LUT(img, lookUpTable, out);
        return out;
    }

    cv::Rect ExtractBiggestFeature(const cv::Mat& img, uint16_t horizontalDilation)
    {
        cv::Mat blurred{};

        cv::GaussianBlur(img, blurred, cv::Size{15, 15}, 0);

        cv::dilate(blurred, blurred, cv::Mat::ones(1, horizontalDilation, CV_8UC1), cv::Point(-1, -1));
        cv::threshold(cv::Scalar(255) - blurred, blurred, 50, 255, cv::THRESH_BINARY_INV);

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(blurred, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

        std::sort(contours.begin(), contours.end(), [](const std::vector<cv::Point>& firstContour,
                                                       const std::vector<cv::Point>& secondContour) {
            return cv::contourArea(firstContour) < cv::contourArea(secondContour);
        });

        std::vector<cv::Point> largestContour = contours[contours.size() - 1];
        double contoursArcLength = cv::arcLength(largestContour, true);
        std::vector<cv::Point> approximation;
        cv::approxPolyDP(largestContour, approximation, 0.02 * contoursArcLength, true);

        cv::Rect boudingBox = cv::boundingRect(approximation);

        return boudingBox;
    }
}
