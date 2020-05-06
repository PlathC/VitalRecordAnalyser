//
// Created by Platholl on 06/05/2020.
//

#include "DatasetBuilder/ImageSet.hpp"

namespace DatasetBuilder{
    ImageSet::ImageSet(const std::string& m_path, const std::string& outputFolder) :
        m_folderPath(m_path),
        m_outputFolder(outputFolder){
        // TODO: Open all images file
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
            top.Save(m_folderPath);
        }
        m_images.pop();

        return m_images.size();
    }
}