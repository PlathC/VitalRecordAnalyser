//
// Created by Platholl on 07/05/2020.
//

#include "Model/TextSegmentation/TextSegmentation.hpp"

TextSegmentation::TextSegmentation(const std::string& src) :
    m_src(cv::imread(src))
{
}

TextSegmentation::TextSegmentation(TextSegmentation&& oldTextSegmentation) noexcept:
        m_progress(oldTextSegmentation.m_progress),
        m_src(oldTextSegmentation.m_src),
        m_thread(std::move(oldTextSegmentation.m_thread))
{
}

TextSegmentation& TextSegmentation::operator=(TextSegmentation&& oldTextSegmentation) noexcept
{
    m_progress = oldTextSegmentation.m_progress;
    m_src = oldTextSegmentation.m_src;

    oldTextSegmentation.m_thread.swap(m_thread);
    return *this;
}

void TextSegmentation::Start()
{
    m_thread = std::thread([&](){
        this->Process();
    });
}

void TextSegmentation::Join()
{
    m_thread.join();
}

uint8_t TextSegmentation::Progress()
{
    std::lock_guard lock{m_progressLocker};
    return m_progress;
}

std::vector<std::vector<cv::Mat>> TextSegmentation::GetExtractedWords()
{
    std::lock_guard lock{m_imagesLock};
    return m_extractedWords;
}

void TextSegmentation::Process()
{
    std::vector<std::vector<cv::Mat>> detectedWords;

    auto updateProgressValue = [&](int newValue){
        std::lock_guard lock{m_progressLocker};
        m_progress = newValue;
    };

    // Preprocessing
    cv::Mat img = m_src.clone();
    if(img.channels() > 1)
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);

    cv::Rect sheetsBB = preprocessing::ExtractBiggestFeature(img);
    img = img(sheetsBB);
    img = preprocessing::GammaCorrection(img);
    updateProgressValue(10);

    img = preprocessing::LocallySoftAdaptiveBinarization(img);
    updateProgressValue(30);

    auto quarters = segmentation::SegmentCivilStates(img);
    updateProgressValue(45);

    pybind11::gil_scoped_acquire acquire;
    segmentation::EASTDetector detector{};
    //auto visu = m_src.clone();

    const auto step = static_cast<uint8_t>(55. / quarters.size());
    for(const auto& quarter : quarters)
    {
        cv::Mat working = img(quarter).clone();

        cv::cvtColor(working, working, cv::COLOR_GRAY2BGR);
        auto boxes = detector.Process(working);

        std::vector<cv::Mat> words;
        words.reserve(boxes.size());
        std::reverse(std::begin(boxes), std::end(boxes));

        for(const auto& box : boxes)
        {
            words.emplace_back(m_src(box + cv::Point(quarter.x, quarter.y) + cv::Point(sheetsBB.x, sheetsBB.y)));
            //cv::rectangle(visu, cv::Point{sheetsBB.x + box.x + quarter.x, sheetsBB.y + box.y + quarter.y},
            //              cv::Point{sheetsBB.x + box.x + box.width + quarter.x, sheetsBB.y + box.y + box.height + quarter.y}, cv::Scalar(0, 0, 0), 5);
        }
        detectedWords.emplace_back(words);
        updateProgressValue(m_progress + step);
    }
    updateProgressValue(m_progress + step);

    // cv::imwrite("test.png", visu);
    // double ratio = 720. / visu.rows;
    // int newWidth = visu.cols * ratio;
    // cv::resize(visu, visu, cv::Size(newWidth, 720));
    // cv::imshow("", visu);
    // cv::waitKey(0);

    // Handle detection directiob to provide the detection in a correct left to right / up to down way
    //std::reverse(std::begin(detectedWords), std::end(detectedWords));
    //std::reverse(std::begin(detectedWords), std::end(detectedWords));

    std::lock_guard lockImg{m_imagesLock};
    m_extractedWords = detectedWords;

    updateProgressValue(100);
}

TextSegmentation::~TextSegmentation()
{
    if(m_thread.joinable())
        m_thread.join();
}