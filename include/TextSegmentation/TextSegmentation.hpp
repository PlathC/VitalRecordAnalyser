//
// Created by Platholl on 07/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_TEXTSEGMENTATION_HPP
#define CIVILREGISTRYANALYSER_TEXTSEGMENTATION_HPP

#include <filesystem>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "TextSegmentation/Binarization.hpp"
#include "TextSegmentation/Scanner.hpp"
#include "TextSegmentation/LineSegmentation.hpp"
#include "TextSegmentation/WordSegmentation.hpp"


namespace fs = std::filesystem;

class TextSegmentation
{
public:
    TextSegmentation(const std::string& src, const std::string& out);

    std::vector<cv::Mat> ExtractWords();

private:
    cv::Mat m_src;
    std::string m_outputPath;
    std::string m_name;
    std::string m_extension;

    fs::path m_wordsPath;
};

#endif //CIVILREGISTRYANALYSER_TEXTSEGMENTATION_HPP
