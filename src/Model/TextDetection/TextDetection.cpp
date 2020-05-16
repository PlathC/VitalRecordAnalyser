//
// Created by Platholl on 15/05/2020.
//

#include "Model/TextDetection/TextDetection.hpp"

TextDetection::TextDetection()
{
    py::exec(R"(
        import sys
        sys.path.insert(0,'py')
    )");
    textDetection = py::module::import("text_detection");
}

std::string TextDetection::Process(const cv::Mat& src)
{
    try
    {
        py::object result = textDetection.attr("read_text_from_image")(src);
        std::string output = result.cast<std::string>();
        return output;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what();
        return "";
    }
}
