//
// Created by enzob on 21/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_TEXTCORRECTION_HPP
#define CIVILREGISTRYANALYSER_TEXTCORRECTION_HPP

#include <iostream>
#include <stdexcept>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;

class TextCorrection {
public:
    TextCorrection();
    std::string Correct(const std::string& sentence);

private:
    py::module textCorrection;
};


#endif //CIVILREGISTRYANALYSER_TEXTCORRECTION_HPP
