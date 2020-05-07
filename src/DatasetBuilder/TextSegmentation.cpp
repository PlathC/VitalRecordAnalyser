//
// Created by Platholl on 07/05/2020.
//

#include "include/TextSegmentation/TextSegmentation.hpp"

TextSegmentation::TextSegmentation(const std::string& src, const std::string& out) :
    m_src(cv::imread(src)),
    m_outputPath(out)
{
    m_name = m_outputPath.substr(m_outputPath.find_last_of("/\\") + 1);
    m_name = m_name.substr(0, m_name.find("."));

    m_extension = ".png";
    m_wordsPath = m_outputPath;
    m_wordsPath /= "words";

    fs::create_directories(m_outputPath);
}

std::vector<cv::Mat> TextSegmentation::Process()
{
    std::vector<cv::Mat> detectedWords;
    cv::Mat blurred;
    cv::blur(m_src, blurred, cv::Point(11, 11));

    cv::Mat edges;
    cv::Canny(blurred, edges, 0, 200);
    cv::threshold(edges, edges, 220, 255, cv::ThresholdTypes::THRESH_BINARY);
    cv::imwrite(m_outputPath + "/" + "threshold.png", edges);

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

    cv::imwrite(m_outputPath + "/" + "FULLCONTOUR.png", fullContourImg);

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
    cv::imwrite(m_outputPath + "/" + "CONTOURS.png", contourImg);

    int i = 0;
    for(const auto& img : detectedArea)
    {
        cv::imwrite(m_outputPath + "/" + std::to_string(i) + ".png", img);
        std::cout << "Writing into " + m_outputPath + "/" << std::endl;
        std::cout << std::to_string(i++) << "/" << detectedArea.size() << std::endl;

        std::vector<cv::Mat> extracted = ExtractWords(img);
        detectedWords.insert(detectedWords.begin(), extracted.begin(), extracted.end());
    }

    return detectedWords;
}

std::vector<cv::Mat> TextSegmentation::ExtractWords(const cv::Mat& src)
{
    // START Step 1: crop //
    std::unique_ptr<Scanner> scanner = std::make_unique<Scanner>();
    cv::Mat imageCropped;
    scanner->process(src, imageCropped);

    fs::path saveCrop = m_outputPath;
    std::string cropName = m_name + "_1_crop" + m_extension;
    saveCrop /= cropName;
    cv::imwrite(saveCrop.u8string(), imageCropped);
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
    threshold->binarize(imageCropped, imageBinary, true, 4);

    fs::path saveBinary = m_outputPath;
    std::string binaryName = m_name + "_2_binary" + m_extension;
    saveBinary /= binaryName;
    cv::imwrite(saveBinary.u8string(), imageBinary);
    // END Step 2 //


    // START Step 3: line segmentation //
    std::unique_ptr<LineSegmentation> line = std::make_unique<LineSegmentation>();
    std::vector<cv::Mat> lines;
    cv::Mat imageLines = imageBinary.clone();
    line->segment(imageLines, lines, chunksNumber, chunksProcess);

    fs::path saveLines = m_outputPath;
    std::string linesName = m_name + "_3_lines" + m_extension;
    saveLines /= linesName;
    cv::imwrite(saveLines.u8string(), imageLines);
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

        fs::create_directories(m_wordsPath);
        for (int j=0; j<words.size(); j++) {
            std::string wordIndex = lineIndex + "_" + std::to_string((j+1)*1e-6).substr(5);
            fs::path saveWord = m_wordsPath / (wordIndex + m_extension);
            cv::imwrite(saveWord.u8string(), words[j]);
            wordsSave.push_back(words[j]);
        }
    }

    //fs::path saveSeparateLine = m_outputPath;
//
    //for (int i=0; i<summary.size(); i++){
    //    std::string index = "_4_summary_" + std::to_string((i+1)*1e-6).substr(5);
    //    std::string separateLineName = m_name + index + m_extension;
    //    fs::path saveLine = saveSeparateLine / separateLineName;
    //    cv::imwrite(saveLine.u8string(), summary[i]);
    //}
    // END Step 4 //

    return wordsSave;
}