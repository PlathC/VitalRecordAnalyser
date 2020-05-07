#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

class Scanner {
    public:
        Scanner();
        void process(cv::Mat image, cv::Mat &output);
	    bool cropped;

    private:
        void processEdge(cv::Mat input, cv::Mat &output, int openKSize, int closeKSize, bool gaussianBlur);
        void fourPointTransform(cv::Mat src, cv::Mat &dst, std::vector<cv::Point> pts);
        void orderPoints(std::vector<cv::Point> inpts, std::vector<cv::Point> &ordered);
};
