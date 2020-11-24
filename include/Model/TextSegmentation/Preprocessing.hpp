#ifndef CIVILREGISTRYANALYSER_PREPROCESSING_HPP
#define CIVILREGISTRYANALYSER_PREPROCESSING_HPP

#include <string>

#include <opencv2/opencv.hpp>

namespace segmentation
{
    class Preprocessing
    {
    public:
        Preprocessing();

        void Perform(const std::string& fileName);

    private:

    };


    static void GammaCorrection(cv::Mat& inout, float gamma=2);
    static void LocallyAdaptiveSoftBinarisation(cv::Mat& inout);
}

#endif //CIVILREGISTRYANALYSER_PREPROCESSING_HPP
