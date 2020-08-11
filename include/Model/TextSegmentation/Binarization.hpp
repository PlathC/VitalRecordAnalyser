#include <iostream>

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"


class Binarization {
    public:
        Binarization();
        void binarize(const cv::Mat& image, cv::Mat &output, bool light, int option);

        float hr;
        cv::Mat histogram;
        cv::Mat cei;
        cv::Mat ceiBin;
        cv::Mat egAvg;
        cv::Mat egBin;
        cv::Mat tli;
        cv::Mat tliErosion;
        cv::Mat ldi;

    private:
        void getHistogram(const cv::Mat& image);
        void getHR(float sqrtHW);
        void getCEI(const cv::Mat& grayscale);
        void getEdge(const cv::Mat& grayscale);
        void getTLI(const cv::Mat& grayscale);

        void lightDistribution(cv::Mat &grayscale);
        void thresholdImg(cv::Mat im, cv::Mat &output, int option, int winx, int winy, double k, double dR);

        double calcLocalStats(cv::Mat &im, cv::Mat &mapM, cv::Mat &mapS, int winx, int winy);
		cv::Mat scale(const cv::Mat& image);
};
