//
// Created by Platholl on 06/05/2020.
//

#include "DatasetBuilder/DatasetImage.hpp"

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

    void DatasetImage::Save(const std::string& outputFolder) const
    {
        std::string fileName = outputFolder + "/" + m_name;
        cv::imwrite(fileName + ".png", m_src);

        std::ofstream outfile (fileName + ".txt");
        outfile << m_text << std::endl;
        outfile.close();
    }

    void DatasetImage::Text(const std::string& nText) { m_text = nText; }

    const cv::Mat& DatasetImage::Image() const { return m_src; }

}