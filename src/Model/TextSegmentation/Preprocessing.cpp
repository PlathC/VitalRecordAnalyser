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

    cv::Mat ExtractBiggestFeature(const cv::Mat& img, uint16_t horizontalDilation)
    {
        cv::Mat blurred{};
        cv::GaussianBlur(img, blurred, cv::Size{15, 15}, 0);

        cv::Mat element = getStructuringElement(cv::MORPH_ELLIPSE,
                                                cv::Size(1, horizontalDilation));
        cv::dilate(blurred, blurred, element);
        element = getStructuringElement(cv::MORPH_ELLIPSE,
                                        cv::Size(horizontalDilation, horizontalDilation));
        cv::erode(blurred, blurred, element);

        cv::threshold(blurred, blurred, 50, 255, cv::THRESH_BINARY_INV);

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(blurred, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

        std::sort(contours.begin(), contours.end(), [](std::vector<cv::Point> firstContour,
                                                       std::vector<cv::Point> secondContour) {
            return cv::contourArea(firstContour) < cv::contourArea(secondContour);
        });

        std::vector<cv::Point> largestContour = contours[contours.size() - 2];
        double contoursArcLength = cv::arcLength(largestContour, true);
        std::vector<cv::Point> approximation;
        cv::approxPolyDP(largestContour, approximation, 0.02 * contoursArcLength, true);

        cv::Rect boudingBox = cv::boundingRect(approximation);

        return img(boudingBox).clone();
    }
}
