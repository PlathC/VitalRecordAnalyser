#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

class Scanner {
    public:
        Scanner();
        void process(cv::Mat image, cv::Mat &output);
	    bool cropped;

    private:
        void processEdge(const cv::Mat& input, cv::Mat &output, int openKSize, int closeKSize, bool gaussianBlur);
        void fourPointTransform(const cv::Mat& src, cv::Mat &dst, const std::vector<cv::Point>& pts);
        void orderPoints(const std::vector<cv::Point>& inpts, std::vector<cv::Point> &ordered);
};
