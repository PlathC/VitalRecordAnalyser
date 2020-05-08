//
// Created by Platholl on 06/05/2020.
//

#include "DatasetBuilder/ImageSet.hpp"

namespace DatasetBuilder{
    const std::array<std::string, 3> ImageSet::SupportedImageFiles = {
            ".jpg", ".jpeg", ".png"
    };

    ImageSet::ImageSet(const std::string& m_path, const std::string& outputFolder) :
        m_folderPath(m_path),
        m_outputFolder(outputFolder)
    {
        std::string workingDir = m_outputFolder + "/temp";
        fs::create_directory(workingDir);

        fs::path file {m_path};
        if(file.has_filename())
        {
            bool isFileSupported = std::find(
                    SupportedImageFiles.begin(),
                    SupportedImageFiles.end(),
                    file.extension().string())
                            != SupportedImageFiles.end();
            if(isFileSupported)
            {
                TextSegmentation segmentor = TextSegmentation(file.string(), workingDir);
                std::vector<cv::Mat> imageList;// = segmentor.Process();
                for(const auto& image : imageList)
                {
                    m_images.push(DatasetImage(image, outputFolder));
                }
            }
        }
    }

    ImageSet::ImageSet(const std::vector<cv::Mat>& images, const std::string& outputFolder):
            m_outputFolder(outputFolder)
    {
        for(const auto& img: images)
        {
            m_images.push(DatasetImage(img, outputFolder));
        }
    }

    DatasetImage& ImageSet::CurrentImage()
    {
        return m_images.top();
    }

    int ImageSet::Skip(bool save)
    {
        auto& top = m_images.top();
        if(save)
        {
            top.Save(m_outputFolder);
        }
        m_images.pop();

        return m_images.size();
    }

    const std::string& ImageSet::FolderPath() const
    {
        return m_folderPath;
    }

    const std::string& ImageSet::OutputFolder() const
    {
        return m_outputFolder;
    }

    void ImageSet::SetOutputFolder(const std::string& folder)
    {
        m_outputFolder = folder;
    }
}