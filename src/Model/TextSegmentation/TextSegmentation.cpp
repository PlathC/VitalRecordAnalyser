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

    m_thread = std::move(oldTextSegmentation.m_thread);
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

std::vector<cv::Mat> TextSegmentation::GetExtractedWords()
{
    std::lock_guard lock{m_imagesLock};
    return m_processedImages;
}

void TextSegmentation::Process()
{
    std::vector<cv::Mat> detectedWords;
    cv::Mat blurred;
    cv::blur(m_src, blurred, cv::Point(11, 11));

    cv::Mat edges;
    cv::Canny(blurred, edges, 0, 200);
    cv::threshold(edges, edges, 220, 255, cv::ThresholdTypes::THRESH_BINARY);

    int kernelSize = 2;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2 * kernelSize + 1, 2 * kernelSize + 1),
                                                cv::Point(kernelSize, kernelSize) );
    cv::Mat dilation;
    cv::dilate(edges, dilation, element, cv::Point(-1,-1), 1);
    cv::erode(dilation, dilation, element, cv::Point(-1,-1), 1);
    cv::dilate(dilation, dilation, element, cv::Point(-1,-1), 35);
    cv::erode(dilation, dilation, element, cv::Point(-1,-1), 5);

    cv::Mat dilationEdges;
    cv::Canny(dilation, dilationEdges, 100, 200);

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(dilationEdges, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Mat fullContourImg;
    m_src.copyTo(fullContourImg);

    for(int i = 0; i < contours.size(); i++)
        cv::drawContours( fullContourImg, contours, i, cv::Scalar(0, 0, 0), 2, 8, hierarchy, 0, cv::Point() );

    cv::Mat contourImg;
    m_src.copyTo(contourImg);
    std::vector<cv::Mat> detectedArea;
    for(size_t i = 0; i < contours.size(); i++)
    {
        auto& currentContour = contours[i];
        if(currentContour.empty())
            continue;

        if (cv::contourArea(currentContour) < MinArea)
            continue;

        cv::Rect box = cv::boundingRect(currentContour);
        cv::Mat out = m_src(box);
        cv::rectangle(contourImg,
                cv::Point(box.x, box.y),
                cv::Point( box.x + box.width, box.y + box.height),
                cv::Scalar(0, 255, 0), 4),
        detectedArea.push_back(out);
    }

    m_progressLocker.lock();
    m_progress = 20;
    m_progressLocker.unlock();

    int i = 0;
    detectedArea.erase(std::remove_if(detectedArea.begin(), detectedArea.end(), [](const auto& img ) -> bool
    {
        return (img.cols * 2) < img.rows;
    }), detectedArea.end());


    uint8_t step = 80 / detectedArea.size();
    for(const auto& img : detectedArea)
    {
        std::vector<cv::Mat> extracted = ExtractWords(img);

        detectedWords.insert(detectedWords.begin(), extracted.begin(), extracted.end());

        std::lock_guard lock{m_progressLocker};
        m_progress += step;
    }

    std::lock_guard lockImg{m_imagesLock};
    m_processedImages.insert(m_processedImages.begin(), detectedWords.begin(), detectedWords.end());
    std::lock_guard lockProgress{m_progressLocker};
    m_progress = 100;
}

std::vector<cv::Mat> TextSegmentation::ExtractWords(const cv::Mat& src)
{
    // START Step 1: crop //
    std::unique_ptr<Scanner> scanner = std::make_unique<Scanner>();
    cv::Mat imageCropped;
    scanner->process(src, imageCropped);
    // END Step 1 //

    // START Step 1.1: resize and definitions //
    int newW = 1280;
    int newH = ((newW * imageCropped.rows) / imageCropped.cols);
    cv::resize(imageCropped, imageCropped, cv::Size(newW, newH));

    int chunksNumber = 8;
    int chunksProcess = 4;
    // END Step 1.1 //

    // START Step 2: binarization //
    std::unique_ptr<Binarization> threshold = std::make_unique<Binarization>();
    cv::Mat imageBinary;

    // default = 0 | otsu = 1 | niblack = 2 | sauvola = 3 | wolf = 4 //
    threshold->binarize(imageCropped, imageBinary, false, 0);
    // END Step 2 //

    // START Step 3: line segmentation //
    std::unique_ptr<LineSegmentation> line = std::make_unique<LineSegmentation>();
    std::vector<cv::Mat> lines;
    cv::Mat imageLines = imageBinary.clone();
    line->segment(imageLines, lines, chunksNumber, chunksProcess);
    // END Step 3 //

    // START Step 4: word segmentation //
    std::unique_ptr<WordSegmentation> word = std::make_unique<WordSegmentation>();
    std::vector<cv::Mat> summary;
    std::vector<cv::Mat> wordsSave;
    word->setKernel(11, 5, 1);

    for (int i=0; i<lines.size(); i++) {
        std::string lineIndex = std::to_string((i+1)*1e-6).substr(5);

        std::vector<cv::Mat> words;
        word->segment(lines[i], words);

        summary.push_back(words[0]);
        words.erase(words.begin());

        words.erase(std::remove_if(words.begin(), words.end(), [](const auto& img ) -> bool
        {
            return img.cols < 100;
        }), words.end());

        for (const auto & currentWord : words)
            wordsSave.push_back(currentWord);

    }

    return wordsSave;
}