//
// Created by Platholl on 15/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_TEXTDETECTION_HPP
#define CIVILREGISTRYANALYSER_TEXTDETECTION_HPP

#include <iostream>
#include <stdexcept>

#include <opencv2/opencv.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "MatCaster.hpp"

namespace py = pybind11;

class TextDetection
{
public:
    TextDetection();
    std::string Process(const cv::Mat& src);
    std::string Correct(const std::string &sentence);
private:
    py::scoped_interpreter interpreter{};
    py::module textDetection;
    py::module textCorrection;
};


#endif //CIVILREGISTRYANALYSER_TEXTDETECTION_HPP
