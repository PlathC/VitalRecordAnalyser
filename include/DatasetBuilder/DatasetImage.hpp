//
// Created by Platholl on 06/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_DATASETIMAGE_HPP
#define CIVILREGISTRYANALYSER_DATASETIMAGE_HPP

#include <string>
#include <filesystem>
#include <fstream>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

namespace fs = std::filesystem;

namespace DatasetBuilder
{
    class DatasetImage
    {
    public:
        explicit DatasetImage(const std::string& path);
        void Save(const std::string& outputFolder) const;

        void Text(const std::string& nText);
        const cv::Mat& Image() const;
    private:
        cv::Mat m_src;
        fs::path m_srcPath;
        std::string m_text;
    };
}



#endif //CIVILREGISTRYANALYSER_DATASETIMAGE_HPP
