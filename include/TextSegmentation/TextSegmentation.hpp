//
// Created by Platholl on 07/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_TEXTSEGMENTATION_HPP
#define CIVILREGISTRYANALYSER_TEXTSEGMENTATION_HPP

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
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

    TextSegmentation(const TextSegmentation&) = delete;
    TextSegmentation& operator=(const TextSegmentation&) = delete;

    TextSegmentation(TextSegmentation&&) noexcept;
    TextSegmentation& operator=(TextSegmentation&&) noexcept;

    void Start();
    void Join();
    uint8_t Progress();
    void Process();
    std::vector<cv::Mat> GetExtractedWords();
    std::vector<cv::Mat> ExtractWords(const cv::Mat& src);

private:
    uint8_t m_progress = 0;
    std::mutex m_progressLocker;

    std::vector<cv::Mat> m_processedImages;
    std::mutex m_imagesLock;

    std::thread m_thread;

    static constexpr uint16_t MinArea = 44000;
    cv::Mat m_src;
    std::string m_outputPath;
    std::string m_name;
    std::string m_extension;

    fs::path m_wordsPath;
};

#endif //CIVILREGISTRYANALYSER_TEXTSEGMENTATION_HPP
