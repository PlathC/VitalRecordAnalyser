#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

class WordSegmentation {
public:
    WordSegmentation();

    void segment(cv::Mat line, std::vector<cv::Mat> &words);
    void setKernel(int kernelSize, int sigma, int theta);

private:
    std::string srcBase;
    std::string extension;
    cv::Mat kernel;

    void printContours(cv::Mat image, std::vector<std::vector<cv::Point>> contours, std::vector<cv::Vec4i> hierarchy, int idx);
    void processBounds(cv::Mat &image, std::vector<cv::Rect> &boundRect);
};
