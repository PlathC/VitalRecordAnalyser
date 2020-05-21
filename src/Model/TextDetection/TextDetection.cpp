//
// Created by Platholl on 15/05/2020.
//

#include "Model/TextDetection/TextDetection.hpp"

TextDetection::TextDetection()
{
    try
    {
        py::exec(R"(
            import sys
            sys.path.insert(0,'py')
        )");
        textDetection = py::module::import("text_detection");
        textCorrection = py::module::import("text_correction");
        textDetection.attr("load_model")();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

std::string TextDetection::Process(const cv::Mat& src)
{
    try
    {
        py::object result = textDetection.attr("read_text_from_image")(src);
        auto output = result.cast<std::string>();
        return output;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what();
        return "";
    }
}


std::string TextDetection::Correct(const std::string &sentence)
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