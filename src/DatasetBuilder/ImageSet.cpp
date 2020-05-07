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
        for(const auto& file : std::filesystem::recursive_directory_iterator(m_path))
        {
            if(file.is_regular_file())
            {
                auto& filePath = file.path();
                bool isFileSupported = std::find(
                        SupportedImageFiles.begin(),
                        SupportedImageFiles.end(),
                        filePath.extension().string())
                                != SupportedImageFiles.end();
                if(isFileSupported)
                {
                    m_images.push(DatasetImage(filePath.string()));
                }
            }
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