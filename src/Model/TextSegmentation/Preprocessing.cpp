#include "Model/TextSegmentation/Preprocessing.hpp"

namespace segmentation
{
    Preprocessing::Preprocessing()
    {
    }

    void Preprocessing::Perform(const std::string &fileName)
    {
    }

    static void GammaCorrection(cv::Mat& inout, float gamma)
    {
        float invGamma = 1.f / gamma;
        std::array<float, 256> lookUpTable{};
        for(size_t i = 0; i < lookUpTable.size(); i++)
        {
            lookUpTable[i] = (std::pow((static_cast<float>(i) / 255.f), invGamma)) * 255;
        }

        cv::LUT(inout, lookUpTable, inout);
    }

}
