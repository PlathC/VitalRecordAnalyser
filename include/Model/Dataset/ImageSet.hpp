//
// Created by Platholl on 06/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_IMAGESET_HPP
#define CIVILREGISTRYANALYSER_IMAGESET_HPP

#include <stack>
#include <string>
#include <filesystem>

#include "Model/TextSegmentation/TextSegmentation.hpp"

#include "DatasetImage.hpp"

namespace fs = std::filesystem;

namespace DatasetBuilder
{
    class ImageSet
    {
    public:
        explicit ImageSet(const std::string& path, const std::string& outputFolder = "");
        explicit ImageSet(const std::string& path, const std::vector<cv::Mat>& images, const std::string& outputFolder = "");

        DatasetImage& CurrentImage();
        std::size_t Skip(bool save);

        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] const fs::path& SourceImage() const;
        [[nodiscard]] const std::string& ImageSet::OutputFolder() const;
        void SetOutputFolder(const std::string& folder);
        void SetTranscriptionPath(const fs::path& file);
    private:
        static const std::array<std::string, 3> SupportedImageFiles;

        fs::path m_inputPath;
        fs::path m_transcriptionPath;
        std::string m_outputFolder;
        std::stack<DatasetImage> m_images;
        cv::Mat m_src;
        bool m_srcSaved = false;
        uint16_t m_imgCount = 0;
    };
}

#endif //CIVILREGISTRYANALYSER_IMAGESET_HPP
