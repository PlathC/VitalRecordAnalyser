//
// Created by Platholl on 06/05/2020.
//

#include "DatasetBuilder/DatasetImage.hpp"

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

    void DatasetImage::Save(const std::string& outputFolder, const fs::path& transcriptionPath, const std::string& originalImgName) const
    {
        std::string fileName = outputFolder + "/" + m_name;
        cv::imwrite(fileName + ".png", m_src);

        std::ifstream iTranscription(transcriptionPath);
        json transcription;
        iTranscription >> transcription;
        iTranscription.close();

        transcription[originalImgName][fs::relative(fileName, outputFolder).string()] = m_text;

        std::ofstream oTranscription(transcriptionPath, std::ofstream::trunc);
        oTranscription << std::setw(4) << transcription << std::endl;
        oTranscription.close();


//        std::ofstream outfile (fileName + ".txt");
//        outfile << m_text << std::endl;
//        outfile.close();
    }

    void DatasetImage::Text(const std::string& nText) { m_text = nText; }

    const cv::Mat& DatasetImage::Image() const { return m_src; }

}