//
// Created by Platholl on 15/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_TEXTDETECTION_HPP
#define CIVILREGISTRYANALYSER_TEXTDETECTION_HPP

#include <iostream>
#include <map>
#include <stdexcept>

#include <opencv2/opencv.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "MatCaster.hpp"

namespace py = pybind11;

class TextDetection
{
public:
    TextDetection();
    std::string Process(const cv::Mat& src);
    std::vector<std::string> Correct(const std::string& text);
    std::map<std::string, std::string> AnalyseText(const std::string& text);
private:
    py::scoped_interpreter interpreter{};
    py::module textDetection;
    py::module textCorrection;
    py::module textAnalyser;
};


#endif //CIVILREGISTRYANALYSER_TEXTDETECTION_HPP
