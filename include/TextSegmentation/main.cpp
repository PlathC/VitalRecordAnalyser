#include "Binarization.hpp"
#include "Scanner.hpp"
#include "LineSegmentation.hpp"
#include "WordSegmentation.hpp"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {

    if (argc < 2)
    {
        std::cout << "This application needs at least 2 arguments." << std::endl;
        return EXIT_SUCCESS;
    }

    std::string srcPath = argv[1];
    std::string outPath = argv[2];

    cv::Mat image = cv::imread(srcPath);

    cv::String name = outPath.substr(outPath.find_last_of("/\\") + 1);
    name = name.substr(0, name.find("."));

    std::string extension = ".png";
    fs::path wordsPath = outPath;
    wordsPath /= "words";

    fs::create_directories(outPath);

    // START Step 1: crop //
    Scanner *scanner = new Scanner();
    cv::Mat imageCropped;
    scanner->process(image, imageCropped);

    fs::path saveCrop = outPath;
    std::string cropName = name + "_1_crop" + extension;
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

    fs::path saveBinary = outPath;
    std::string binaryName = name + "_2_binary" + extension;
    saveBinary /= binaryName;
    cv::imwrite(saveBinary.u8string(), imageBinary);
    // END Step 2 //


    // START Step 3: line segmentation //
    LineSegmentation *line = new LineSegmentation();
    std::vector<cv::Mat> lines;
    cv::Mat imageLines = imageBinary.clone();
    line->segment(imageLines, lines, chunksNumber, chunksProcess);

    fs::path saveLines = outPath;
    std::string linesName = name + "_3_lines" + extension;
    saveLines /= linesName;
    cv::imwrite(saveLines.u8string(), imageLines);
    // END Step 3 //


    // START Step 4: word segmentation //
    WordSegmentation *word = new WordSegmentation();
    std::vector<cv::Mat> summary;
    word->setKernel(11, 5, 1);


    for (int i=0; i<lines.size(); i++) {
        std::string lineIndex = std::to_string((i+1)*1e-6).substr(5);

        std::vector<cv::Mat> words;
        word->segment(lines[i], words);

        summary.push_back(words[0]);
        words.erase(words.begin());

        fs::create_directories(wordsPath);

        for (int j=0; j<words.size(); j++) {
            std::string wordIndex = lineIndex + "_" + std::to_string((j+1)*1e-6).substr(5);
            fs::path saveWord = wordsPath / (wordIndex + extension);
            cv::imwrite(saveWord.u8string(), words[j]);
        }
    }

    fs::path saveSeparateLine = outPath;

    for (int i=0; i<summary.size(); i++){
        std::string index = "_4_summary_" + std::to_string((i+1)*1e-6).substr(5);
        std::string separateLineName = name + index + extension;
        fs::path saveLine = saveSeparateLine / separateLineName;
        cv::imwrite(saveLine.u8string(), summary[i]);
    }
    // END Step 4 //


    return 0;
}