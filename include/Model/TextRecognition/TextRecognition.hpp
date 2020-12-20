//
// Created by Platholl on 15/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_TEXTRECOGNITION_HPP
#define CIVILREGISTRYANALYSER_TEXTRECOGNITION_HPP

#include <iostream>
#include <map>
#include <stdexcept>

#include <opencv2/opencv.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "Model/MatCaster.hpp"

namespace py = pybind11;

class TextRecognition
{
public:
    TextRecognition();
    std::string Process(const cv::Mat& src);
    std::string Correct(const std::string& text);
    std::map<std::string, std::string> AnalyseText(const std::string& text);

private:
    py::module m_textDetection;
    py::module m_textCorrection;
    py::module m_textAnalyser;

    py::object m_nlp;
    py::object m_textRecognizer;
};


#endif //CIVILREGISTRYANALYSER_TEXTRECOGNITION_HPP
