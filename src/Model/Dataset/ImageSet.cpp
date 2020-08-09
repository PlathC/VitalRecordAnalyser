//
// Created by Platholl on 06/05/2020.
//

#include "Model/Dataset/ImageSet.hpp"

namespace DatasetBuilder{
    const std::array<std::string, 3> ImageSet::SupportedImageFiles = {
            ".jpg", ".jpeg", ".png"
    };

    ImageSet::ImageSet(const std::string& path, const std::string& outputFolder) :
            m_inputPath(path),
            m_outputFolder(outputFolder)
    {
        fs::path file {m_inputPath};
        if(file.has_filename())
        {
            bool isFileSupported = std::find(
                    SupportedImageFiles.begin(),
                    SupportedImageFiles.end(),
                    file.extension().string())
                                   != SupportedImageFiles.end();
            if(isFileSupported)
            {
                m_src = cv::imread(file.string());
                TextSegmentation segmentor = TextSegmentation(file.string());
                std::vector<cv::Mat> imageList;// = segmentor.Process();
                for(const auto& image : imageList)
                {
                    m_images.push(DatasetImage(image, outputFolder));
                }
            }
        }
    }

    ImageSet::ImageSet(const std::string& path, const std::vector<cv::Mat>& images, const std::string& outputFolder):
            m_inputPath(path),
            m_src(cv::imread(path)),
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

    std::size_t ImageSet::Skip(bool save)
    {
        auto& top = m_images.top();
        if(save)
        {
            if(!m_srcSaved)
            {
                cv::imwrite(m_outputFolder + '/' + m_inputPath.stem().string() + ".png", m_src);
                fs::create_directory(m_outputFolder + '/' + m_inputPath.stem().string());
                m_srcSaved = true;
            }

            top.Name(std::to_string(m_imgCount++));
            top.Save(m_outputFolder,
                     m_inputPath.stem().string(),
                     m_transcriptionPath,
                     m_outputFolder + '/'+ m_inputPath.stem().string() + ".png");
        }
        m_images.pop();

        return m_images.size();
    }

    std::size_t ImageSet::Size() const
    {
        return m_images.size();
    }

    const fs::path& ImageSet::SourceImage() const
    {
        return m_inputPath;
    }

    const std::string& ImageSet::OutputFolder() const
    {
        return m_outputFolder;
    }

    void ImageSet::SetOutputFolder(const std::string& folder)
    {
        m_outputFolder = folder;
    }

    void ImageSet::SetTranscriptionPath(const fs::path& file)
    {
        m_transcriptionPath = file;
    }
}