#include "Model/TextSegmentation/WordSegmentation.hpp"

WordSegmentation::WordSegmentation() {};

bool compareXCords(const cv::Rect &p1, const cv::Rect &p2)
{
    return (p1.x < p2.x);
}

void WordSegmentation::printContours(cv::Mat image, std::vector<std::vector<cv::Point>> contours, std::vector<cv::Vec4i> hierarchy, int idx)
{
    for(int i = idx; i >= 0; i = hierarchy[i][0])
    {
        drawContours(image, contours, i, cv::Scalar(255));
        for(int j=hierarchy[i][2]; j>=0; j=hierarchy[j][0])
            printContours(image, contours, hierarchy, hierarchy[j][2]);
    }
}

void WordSegmentation::processBounds(cv::Mat &image, std::vector<cv::Rect> &boundRect)
{
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::Mat edged;
    size_t lastNumber = 0;

    while(true)
    {
        cv::findContours(image, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        edged = cv::Mat::zeros(cv::Size(image.cols, image.rows), CV_8UC1);

        for(auto & contour : contours)
        {
            cv::Rect r = boundingRect(cv::Mat(contour));
            cv::rectangle(edged, r.tl(), r.br(), 255, 2, 8, 0);
        }

        if(!hierarchy.empty())
            printContours(edged, contours, hierarchy, 0);

        image = edged;

        if (contours.size() == lastNumber) break;
        lastNumber = contours.size();
    }

    for (auto & contour : contours)
        boundRect.push_back(cv::boundingRect(cv::Mat(contour)));

    sort(boundRect.begin(), boundRect.end(), compareXCords);

    int i = 0;
    if(!boundRect.empty())
    {
        while (i<boundRect.size()-1)
        {
            if (boundRect[i].tl().x <= boundRect[i+1].tl().x &&
                boundRect[i].br().x >= boundRect[i+1].br().x)
            {
                int minX = std::min(boundRect[i].tl().x, boundRect[i+1].tl().x);
                int minY = std::min(boundRect[i].tl().y, boundRect[i+1].tl().y);
                int maxY = std::max(boundRect[i].br().y, boundRect[i+1].br().y);

                int width = std::max(boundRect[i].width, boundRect[i + 1].width);
                int height = std::abs(minY - maxY);

                boundRect[i+1] = cv::Rect(minX, minY, width, height);
                boundRect.erase(boundRect.begin() + i);
                continue;
            }
            ++i;
        }
    }
}

void WordSegmentation::segment(cv::Mat line, std::vector<cv::Mat> &words)
{
    copyMakeBorder(line, line, 10, 10, 10, 10, cv::BORDER_CONSTANT, 255);

    cv::Mat imgFiltered;
    cv::filter2D(line, imgFiltered, CV_8UC1, this->kernel);
    cv::threshold(imgFiltered, imgFiltered, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

    std::vector<cv::Rect> boundRect;
    processBounds(imgFiltered, boundRect);

    cv::Mat imageColor;
    cvtColor(line, imageColor, cv::COLOR_GRAY2BGR);

    for (int i=0; i<boundRect.size(); i++){
        cv::Mat cropped;
        line(boundRect[i]).copyTo(cropped);

        rectangle(imageColor, boundRect[i].tl(), boundRect[i].br(), cv::Vec3b(0,0,255), 2, 8, 0);
        putText(imageColor, std::to_string(i+1), boundRect[i].tl(), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Vec3b(255,0,0), 2);
        words.push_back(cropped);
    }

    words.push_back(imageColor);
    rotate(words.rbegin(), words.rbegin()+1, words.rend());
}

void WordSegmentation::setKernel(int kernelSize, int sigma, int theta)
{
    cv::Mat kernel = cv::Mat::zeros(cv::Size(kernelSize, kernelSize), CV_32F);
    auto sigmaX = static_cast<float>(sigma);
    auto sigmaY = static_cast<float>(sigma) * theta;

    for (int i = 0; i < kernelSize; i++)
    {
        for (int j = 0; j<kernelSize; j++)
        {
            auto x = static_cast<float>(i - (kernelSize / 2.f));
            auto y = static_cast<float>(j - (kernelSize / 2.f));

            float termExp = std::exp((-std::pow(x,2) / (2 * sigmaX)) - (std::pow(y,2) / (2 * sigmaY)));

            double termX = (std::pow(x,2) - std::pow(sigmaX,2)) / (2 * CV_PI * std::pow(sigmaX,5) * sigmaY);
            double termY = (std::pow(y,2) - std::pow(sigmaY,2)) / (2 * CV_PI * std::pow(sigmaY,5) * sigmaX);

            kernel.at<float>(i, j) = static_cast<float>((termX + termY) * termExp);
        }
    }

    this->kernel = kernel / cv::sum(kernel)[0];
}
