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
#include <pybind11/numpy.h>

#include "TextDetection/MatCaster.hpp"

namespace py = pybind11;

class TextDetection
{
public:
    TextDetection();
    std::string Process(const cv::Mat& src);

private:
    py::scoped_interpreter guard{};
    py::module textDetection;
};


#endif //CIVILREGISTRYANALYSER_TEXTDETECTION_HPP
