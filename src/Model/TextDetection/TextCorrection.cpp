//
// Created by enzob on 21/05/2020.
//

#include "include/Model/TextDetection/TextCorrection.hpp"

TextCorrection::TextCorrection()
{
    py::exec(R"(
        import sys
        sys.path.insert(0,'py')
    )");
    textCorrection = py::module::import("text_correction");
}

std::string TextCorrection::Correct(const std::string &sentence)
{
    try
    {
        py::object result = textCorrection.attr("correct_sentence")(sentence);
        auto output = result.cast<std::string>();
        return output;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what();
        return "";
    }
}