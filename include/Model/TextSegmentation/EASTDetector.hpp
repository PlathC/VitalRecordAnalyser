//
// Created by Platholl on 06/12/2020.
//

#ifndef CIVILREGISTRYANALYSER_EASTDETECTOR_HPP
#define CIVILREGISTRYANALYSER_EASTDETECTOR_HPP

#include <opencv2/opencv.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include <lanms/lanms.h>

#include "Model/MatCaster.hpp"

namespace py = pybind11;

namespace segmentation
{
    class EASTDetector
    {
    public:
        EASTDetector();

        std::vector<cv::Rect> Process(const cv::Mat& src);

        ~EASTDetector() = default;

    private:
        py::module m_eastDetectorModule;
        py::object m_eastDetector;
    };
}

#endif //CIVILREGISTRYANALYSER_EASTDETECTOR_HPP
