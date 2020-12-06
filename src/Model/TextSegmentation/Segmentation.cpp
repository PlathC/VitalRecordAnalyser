//
// Created by Platholl on 01/12/2020.
//

#include "Model/TextSegmentation/Segmentation.hpp"

namespace segmentation
{
    std::vector<cv::Rect> SegmentCivilStates(const cv::Mat& rawImage)
    {
        std::vector<cv::Rect> regions{};
        std::vector<std::tuple<cv::Rect, cv::Mat>> quarters = DetectQuarters(rawImage, 15);

        for(auto& quarter : quarters)
        {
            // auto subImg = std::get<1>(quarter);
            // auto visu = subImg.clone();
            // double ratio = 480. / visu.rows;
            // int newWidth = visu.cols * ratio;
            // cv::resize(visu, visu, cv::Size(newWidth, 480));
            // cv::imshow("", visu);
            // cv::waitKey(0);

            regions.emplace_back(std::get<0>(quarter));
        }
        return regions;
    }

    std::vector<std::tuple<cv::Rect, cv::Mat>> DetectQuarters(const cv::Mat& img, const int stepDivider)
    {
        cv::Mat workingImg = img.clone();
        if(img.channels() > 1)
        {
            cv::cvtColor(workingImg, workingImg, cv::COLOR_BGR2GRAY);
        }

        // Axis 0 => width, = 1 => height
        const auto findMinimumMiddleIndex = [stepDivider](const cv::Mat& subImg, int axis, int size) -> int
        {
            int center = static_cast<int>(std::floor(size / 2.));
            int step   = static_cast<int>(std::floor(size / static_cast<double>(stepDivider)));

            std::vector<uint32_t> axisHistogram;
            if(axis == 0)
            {
                for(int i = 0; i < subImg.rows; i++)
                {
                    const auto* pixel = subImg.ptr<uint8_t>(i);
                    for(int j = center - step; j < center + step; j++)
                    {
                        if(i == 0)
                        {
                            axisHistogram.emplace_back(0);
                        }
                        axisHistogram[j - (center - step)] += pixel[j];
                    }
                }
            }
            else
            {
                for(int i = center - step; i < center + step; i++)
                {
                    const auto* pixel = subImg.ptr<uint8_t>(i);
                    axisHistogram.emplace_back(0);
                    for(int j = 0; j < subImg.cols; j++)
                    {
                        axisHistogram[axisHistogram.size()-1] += pixel[j] != 0 ? 1 : 0;
                    }
                }
            }
            auto minimumHistogramValue = std::distance(axisHistogram.begin(),
                                                       std::min_element(axisHistogram.begin(),
                                                                        axisHistogram.end()));
            return center - step + minimumHistogramValue;
        };

        int idx1 = findMinimumMiddleIndex(img, 0, img.rows);

        cv::Rect vHalf1 = cv::Rect{0, 0, img.cols, img.rows - idx1};
        cv::Rect vHalf2 = cv::Rect{0, img.rows - idx1, img.cols, idx1};

        int idx2 = findMinimumMiddleIndex(img(vHalf1), 0, vHalf1.width);
        int idx3 = findMinimumMiddleIndex(img(vHalf2), 0, vHalf2.width);

        std::vector<std::tuple<cv::Rect, cv::Mat>> results{};

        // Upper left corner
        cv::Rect r1 = cv::Rect{0, 0, idx2, vHalf1.height};
        cv::Mat i1  = img(r1);
        results.emplace_back(r1, i1);

        // Bottom right corner
        cv::Rect r2 = cv::Rect{img.cols - idx2, img.rows - idx1, idx2, idx1};
        cv::Mat i2  = img(r2);
        results.emplace_back(r2, i2);

        // Bottom left corner
        cv::Rect r3 = cv::Rect{0, img.rows - idx1, img.cols - idx2, idx1};
        cv::Mat i3  = img(r3);
        results.emplace_back(r3, i3);

        // Upper right corner
        cv::Rect r4 = cv::Rect{img.cols - idx3, 0, idx3, idx1};
        cv::Mat i4  = img(r4);
        results.emplace_back(r4, i4);

        return results;
    }
}