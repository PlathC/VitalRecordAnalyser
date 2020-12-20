//
// Created by Platholl on 15/05/2020.
//

#include "Model/TextRecognition/TextRecognition.hpp"

TextRecognition::TextRecognition()
{
    try
    {
        py::exec(R"(
            import sys
            sys.path.insert(0,'py')
        )");

        m_textDetection = py::module::import("text_recognizer");
        m_textCorrection = py::module::import("text_correction");
        m_textAnalyser = py::module::import("nlp");

        assert(py::hasattr(m_textDetection, "TextRecognizer"));
        assert(py::hasattr(m_textAnalyser, "init_nlp_module"));

        m_textRecognizer = m_textDetection.attr("TextRecognizer")();
        m_nlp = m_textAnalyser.attr("init_nlp_module")();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

std::string TextRecognition::Process(const cv::Mat& src)
{
    assert(py::hasattr(m_textRecognizer, "read_text_from_image"));
    try
    {
        py::object result = m_textRecognizer.attr("read_text_from_image")(src);
        auto output = result.cast<std::string>();
        return output;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return "";
    }
}


std::string TextRecognition::Correct(const std::string& paragraph)
{
    assert(py::hasattr(m_textCorrection, "correct_sentence"));
    try
    {
        return m_textCorrection.attr("correct_sentence")(paragraph).cast<std::string>();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return "";
    }
}

std::map<std::string, std::string> TextRecognition::AnalyseText(const std::string& text)
{
    assert(py::hasattr(m_textAnalyser, "process_text"));
    try
    {
        py::object result = m_textAnalyser.attr("process_text")(m_nlp, text);
        return result.cast<std::map<std::string, std::string>>();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return {};
    }
}