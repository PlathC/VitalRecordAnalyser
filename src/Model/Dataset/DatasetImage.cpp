//
// Created by Platholl on 06/05/2020.
//

#include "Model/Dataset/DatasetImage.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace DatasetBuilder {
    DatasetImage::DatasetImage(const std::string& path) :
            m_src(cv::imread(path))
    {
        auto imgPath = fs::path(path);
        m_name = imgPath.filename().string();
    }

    DatasetImage::DatasetImage(const cv::Mat& src, const std::string& name) :
            m_src(src)
    {
        auto imgPath = fs::path(name);
        m_name = imgPath.filename().string();
    }

    void DatasetImage::Name(const std::string& nName)
    {
        m_name = nName;
    }

    void DatasetImage::Save(const std::string& outputFolder, const std::string& segmentsFolder, const fs::path& transcriptionPath, const std::string& originalImgPath) const
    {
        std::string fileName = outputFolder + "/" + segmentsFolder + "/" + m_name + ".png";
        cv::imwrite(fileName, m_src);

        std::ifstream iTranscription(transcriptionPath);
        json transcription;
        if(iTranscription.peek() != std::ifstream::traits_type::eof())
            iTranscription >> transcription;
        iTranscription.close();

        transcription[fs::relative(originalImgPath, outputFolder).string()][fs::relative(fileName, outputFolder).string()] = m_text;

        std::ofstream oTranscription(transcriptionPath, std::ofstream::trunc);
        oTranscription << std::setw(4) << transcription << std::endl;
        oTranscription.close();
    }

    void DatasetImage::Text(const std::string& nText) { m_text = nText; }

    const cv::Mat& DatasetImage::Image() const { return m_src; }

}