//
// Created by Platholl on 06/12/2020.
//

#include <utility>


#include "Model/TextSegmentation/EASTDetector.hpp"

namespace segmentation {
    EASTDetector::EASTDetector()
    {
        try
        {
            py::exec(R"(
                import sys
                sys.path.insert(0,'py')
            )");

            m_eastDetectorModule = py::module::import("east_detector");
            assert(py::hasattr(m_eastDetector, "EastDetector"));
            m_eastDetector = m_eastDetectorModule.attr("EastDetector")();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    std::vector<cv::Rect> EASTDetector::Process(const cv::Mat& src)
    {
        assert(py::hasattr(m_eastDetector, "detect_text"));
        try
        {
            py::object result = m_eastDetector.attr("detect_text")(src);
            auto output = result.cast<std::vector<std::vector<float>>>();
            //std::vector<std::vector<float>> boxes = lanms::polys2floats(lanms::merge_quadrangle_n9(output.data(), output.size(), 0.2));

            return {};
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return {};
        }
    }

}