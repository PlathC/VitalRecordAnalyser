//
// Created by Platholl on 24/11/2020.
//

#include "Model/TextSegmentation/LocallySoftAdaptativeBinarization.hpp"

namespace preprocessing
{
    cv::Mat LocallySoftAdaptativeBinarization(const cv::Mat& img, const uint16_t blockSize)
    {
        // Strongly based on https://stackoverflow.com/a/57103789/9690046

        cv::Mat gray = img.clone();
        std::cout << gray.depth() << std::endl;
        if(gray.channels() > 1)
        {
            cv::cvtColor(gray, gray, cv::COLOR_BGR2GRAY);
        }
        else if (gray.depth() != CV_8U)
        {
            gray.convertTo(gray, CV_8UC1);
        }

        cv::medianBlur(gray, gray, 9);
        cv::Mat invSrc =  cv::Scalar::all(255) - gray;

        // perform threshold on image block
        cv::Mat outImage = cv::Mat(invSrc.rows, invSrc.cols, CV_8UC1);
        auto getBlockBoundingBox = [blockSize](const cv::Mat& inout, const cv::Point& blockCenterPoint) {
            auto y1 = std::max(0, blockCenterPoint.y - blockSize);
            auto x1 = std::max(0, blockCenterPoint.x - blockSize);
            auto y2 = std::min(inout.rows, blockCenterPoint.y + blockSize);
            auto x2 = std::min(inout.cols, blockCenterPoint.x + blockSize);
            return cv::Rect(x1, y1, x2 - x1, y2 - y1);
        };

        for(int i = 0; i < invSrc.rows; i += blockSize)
        {
            for(int j = 0; j < invSrc.cols; j += blockSize)
            {
                auto rect = getBlockBoundingBox(img, cv::Point(j, i));
                cv::Mat bloc = ApplyAdaptiveMedianThreshold(invSrc(rect));
                bloc.copyTo(outImage(rect));
            }
        }

        auto kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        cv::Mat mask;
        cv::morphologyEx(outImage, mask, cv::MORPH_OPEN, kernel);

        outImage = cv::Mat(img.rows, img.cols, CV_8UC1);
        for(int i = 0; i < invSrc.rows; i += blockSize)
        {
            for(int j = 0; j < invSrc.cols; j += blockSize)
            {
                auto rect = getBlockBoundingBox(img, cv::Point(j, i));
                cv::Mat bloc = CombineAdaptativeBlock(img(rect), mask(rect));
                bloc.copyTo(outImage(rect));
            }
        }

        return outImage;
    }

    cv::Mat ApplyAdaptiveMedianThreshold(const cv::Mat& inout, const uint8_t delta)
    {
        auto computeMedianValue = [](const cv::Mat& img) {
            std::vector<uint8_t> vecFromMat;
            for(int i = 0; i < img.rows; i++)
            {
                const auto* pixel = img.ptr<uint8_t>(i);
                for(int j = 0; j < img.cols; j++)
                {
                    vecFromMat.emplace_back(pixel[j]);
                }
            }

            //std::sort(vecFromMat.begin(), vecFromMat.end());
            std::partial_sort(vecFromMat.begin(), vecFromMat.begin() + (vecFromMat.size() / 2 + 1), vecFromMat.end());

            return vecFromMat[vecFromMat.size() / 2];
        };

        double medianValue = computeMedianValue(inout);
        cv::Mat imgOut = cv::Mat(inout.rows, inout.cols, inout.type());
        for(int i = 0; i < inout.rows; i++)
        {
            auto* currentRow = inout.ptr<uint8_t>(i);
            for(int j = 0; j < inout.cols; j++)
            {
                if(currentRow[j] - medianValue < delta)
                {
                    imgOut.ptr<uint8_t>(i)[j] = 255;
                }else
                {
                    imgOut.ptr<uint8_t>(i)[j] = 0;
                }
            }
        }

        cv::dilate(cv::Scalar::all(255) - imgOut, imgOut, cv::Mat(), cv::Point(-1, -1), 9);
        return cv::Scalar::all(255) - imgOut;
    }

    cv::Mat CombineAdaptativeBlock(const cv::Mat& origin, const cv::Mat& mask)
    {
        cv::Mat imgOut = cv::Mat(origin.rows, origin.cols, CV_8UC1);
        std::vector<cv::Point> foregroundIndices;

        cv::Mat floatOriginal;
        origin.convertTo(floatOriginal, CV_32F);
        std::vector<float> selectedPixels;
        for(int i = 0; i < imgOut.rows; i++)
        {
            auto* currentRow = mask.ptr<uint8_t>(i);
            for(int j = 0; j < imgOut.cols; j++)
            {
                if(currentRow[j] == 255)
                {
                    imgOut.ptr<uint8_t>(i)[j] = 255;
                }
                if(currentRow[j] != 255)
                {
                    foregroundIndices.emplace_back(j, i);
                    selectedPixels.emplace_back(floatOriginal.ptr<float>(i)[j]);
                }
            }
        }

        if(foregroundIndices.empty())
        {
            return imgOut;
        }

        // We find the intensity range of our pixels in this local part
        // and clip the image block to that range, locally.
        float lowestValue  = *std::min_element(selectedPixels.begin(), selectedPixels.end());
        float highestValue = *std::max_element(selectedPixels.begin(), selectedPixels.end());
        std::vector<uint8_t> localSelectedPixels = std::vector<uint8_t>(selectedPixels.size());
        for(int i = 0; i < localSelectedPixels.size(); i++)
        {
            localSelectedPixels[i] = selectedPixels[i] - lowestValue;
        }

        float range = highestValue - lowestValue;

        // Now we use good old OTSU binarization to get a rough estimation
        // of foreground and background regions.
        //cv::Mat thresholdedBase = cv::Mat(selectedPixels, CV_8UC1);
        cv::Mat thresholdedBase = cv::Mat( 1, selectedPixels.size(), CV_8UC1, selectedPixels.data());

        cv::Mat thresholded;
        cv::threshold(thresholdedBase, thresholded, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);

        // Then we normalize the stuffs and apply sigmoid to gradually
        // combine the stuffs.
        float boundValue = selectedPixels[0];
        for(int i = 0; i < selectedPixels.size(); i++)
        {
            if(thresholded.ptr<uint8_t>(0)[i] == 255)
            {
                if(selectedPixels[i] < boundValue)
                    boundValue = selectedPixels[i];
            }
        }

        boundValue = (boundValue - lowestValue) / (range + 1e-5f);
        std::vector<float> normalizeFunction = std::vector<float>(localSelectedPixels.size());
        for(uint8_t& localSelectedPixel : localSelectedPixels)
        {
            float newValue = static_cast<float>(localSelectedPixel) / (range + 1e-5f);
            newValue = static_cast<float>(std::exp((newValue - boundValue + 0.05) * 5 / 0.2));
            newValue = newValue / (newValue + 1.f);
            localSelectedPixel = static_cast<uint8_t>(std::floor(newValue));
        }

        // Finally, we re-normalize the result to the range [0..255]
        size_t normalizingIterator = 0;
        for(const auto& index : foregroundIndices)
        {
            imgOut.ptr<uint8_t>(index.y)[index.x] = 255 * localSelectedPixels[normalizingIterator++];
        }

        return imgOut;
    }
}