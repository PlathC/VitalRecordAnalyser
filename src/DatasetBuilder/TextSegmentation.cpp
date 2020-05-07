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

std::vector<cv::Mat> TextSegmentation::ExtractWords()
{
    // START Step 1: crop //
    Scanner *scanner = new Scanner();
    cv::Mat imageCropped;
    scanner->process(m_src, imageCropped);

    fs::path saveCrop = m_outputPath;
    std::string cropName = m_name + "_1_crop" + m_extension;
    saveCrop /= cropName;
    cv::imwrite(saveCrop.u8string(), imageCropped);
    // END Step 1 //


    // START Step 1.1: resize and definitions //
    int newW = 1280;
    int newH = ((newW * imageCropped.rows) / imageCropped.cols);
    cv::resize(imageCropped, imageCropped, cv::Size(newW, newH));

    int chunksNumber = 32;
    int chunksProcess = 16;
    // END Step 1.1 //


    // START Step 2: binarization //
    Binarization *threshold = new Binarization();
    cv::Mat imageBinary;
    // default = 0 | otsu = 1 | niblack = 2 | sauvola = 3 | wolf = 4 //
    threshold->binarize(imageCropped, imageBinary, true, 4);

    fs::path saveBinary = m_outputPath;
    std::string binaryName = m_name + "_2_binary" + m_extension;
    saveBinary /= binaryName;
    cv::imwrite(saveBinary.u8string(), imageBinary);
    // END Step 2 //


    // START Step 3: line segmentation //
    LineSegmentation *line = new LineSegmentation();
    std::vector<cv::Mat> lines;
    cv::Mat imageLines = imageBinary.clone();
    line->segment(imageLines, lines, chunksNumber, chunksProcess);

    fs::path saveLines = m_outputPath;
    std::string linesName = m_name + "_3_lines" + m_extension;
    saveLines /= linesName;
    cv::imwrite(saveLines.u8string(), imageLines);
    // END Step 3 //


    // START Step 4: word segmentation //
    WordSegmentation *word = new WordSegmentation();
    std::vector<cv::Mat> summary;
    std::vector<cv::Mat> wordsSave;
    word->setKernel(11, 5, 1);


    for (int i=0; i<lines.size(); i++) {
        std::string lineIndex = std::to_string((i+1)*1e-6).substr(5);

        std::vector<cv::Mat> words;
        word->segment(lines[i], words);
        wordsSave.insert(wordsSave.begin(), words.begin(), words.end());

        summary.push_back(words[0]);
        words.erase(words.begin());

        fs::create_directories(m_wordsPath);

        for (int j=0; j<words.size(); j++) {
            std::string wordIndex = lineIndex + "_" + std::to_string((j+1)*1e-6).substr(5);
            fs::path saveWord = m_wordsPath / (wordIndex + m_extension);
            cv::imwrite(saveWord.u8string(), words[j]);
        }
    }

    fs::path saveSeparateLine = m_outputPath;

    for (int i=0; i<summary.size(); i++){
        std::string index = "_4_summary_" + std::to_string((i+1)*1e-6).substr(5);
        std::string separateLineName = m_name + index + m_extension;
        fs::path saveLine = saveSeparateLine / separateLineName;
        cv::imwrite(saveLine.u8string(), summary[i]);
    }
    // END Step 4 //

    return wordsSave;
}