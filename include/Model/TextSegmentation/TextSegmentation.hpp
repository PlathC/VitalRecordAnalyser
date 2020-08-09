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

#include "Binarization.hpp"
#include "Scanner.hpp"
#include "LineSegmentation.hpp"
#include "WordSegmentation.hpp"

namespace fs = std::filesystem;

class TextSegmentation
{
public:
    explicit TextSegmentation(const std::string& src);

    TextSegmentation(const TextSegmentation&) = delete;
    TextSegmentation& operator=(const TextSegmentation&) = delete;

    TextSegmentation(TextSegmentation&&) noexcept;
    TextSegmentation& operator=(TextSegmentation&&) noexcept;

    void Start();
    void Join();
    uint8_t Progress();
    void Process();
    std::vector<std::vector<cv::Mat>> GetExtractedWords();
    std::vector<cv::Mat> ExtractWords(const cv::Mat& src);

private:
    uint8_t m_progress = 0;
    std::mutex m_progressLocker;

    std::vector<std::vector<cv::Mat>> m_extractedWords;
    std::mutex m_imagesLock;

    std::thread m_thread;

    static constexpr uint16_t MinArea = 44000;
    cv::Mat m_src;
};

#endif //CIVILREGISTRYANALYSER_TEXTSEGMENTATION_HPP
