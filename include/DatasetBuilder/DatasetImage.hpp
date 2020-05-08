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
        explicit DatasetImage(const cv::Mat& src, const std::string& name);

        void Save(const std::string& outputFolder) const;

        void Name(const std::string& nName);
        void Text(const std::string& nText);
        [[nodiscard]] const cv::Mat& Image() const;
    private:
        cv::Mat m_src;
        std::string m_name;
        std::string m_text;
    };
}



#endif //CIVILREGISTRYANALYSER_DATASETIMAGE_HPP
