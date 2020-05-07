//
// Created by Platholl on 06/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_IMAGESET_HPP
#define CIVILREGISTRYANALYSER_IMAGESET_HPP

#include <stack>
#include <string>

#include "TextSegmentation/TextSegmentation.hpp"

#include "DatasetBuilder/DatasetImage.hpp"

namespace DatasetBuilder
{
    class ImageSet
    {
    public:
        explicit ImageSet(const std::string& path, const std::string& outputFolder = "");

        DatasetImage& CurrentImage();
        int Skip(bool save);

        const std::string& FolderPath() const;
        const std::string& ImageSet::OutputFolder() const;
        void SetOutputFolder(const std::string& folder);
    private:
        static const std::array<std::string, 3> SupportedImageFiles;

        std::string m_folderPath;
        std::string m_outputFolder;
        std::stack<DatasetImage> m_images;
    };
}

#endif //CIVILREGISTRYANALYSER_IMAGESET_HPP
