//
// Created by Platholl on 06/12/2020.
//

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
            assert(py::hasattr(m_eastDetectorModule, "EastDetector"));
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
        assert(py::hasattr(m_eastDetector, "post_process"));
        try
        {
            py::object rawPythonBoxes = m_eastDetector.attr("detect_text")(src);
            auto output = rawPythonBoxes.cast<std::tuple<py::array_t<float,
                                                         py::array::c_style | py::array::forcecast>,
                                                 std::vector<std::vector<float>>,
                                                 std::array<float, 2>>>();
            auto rawBoxes = std::get<0>(output);

            auto pbuf = rawBoxes.request();
            if (pbuf.ndim != 2 || pbuf.shape[1] != 9)
                throw std::runtime_error("quadrangles must have a shape of (n, 9)");

            auto n = pbuf.shape[0];
            auto ptr = static_cast<float *>(pbuf.ptr);

            std::vector<std::vector<float>> nonMixedBoxes = lanms::polys2floats(
                    lanms::merge_quadrangle_n9(ptr, n, 0.2f)
                    );

            py::object finalResult = m_eastDetector.attr("post_process")(nonMixedBoxes,
                    std::get<1>(output), std::get<2>(output), src.cols);

            using Point = std::vector<int>;
            using Box = std::vector<Point>;
            auto finalBoxes = finalResult.cast<std::vector<Box>>();

            std::vector<cv::Rect> resultedBoudingBox{};
            resultedBoudingBox.reserve(finalBoxes.size());
            for(const auto& box : finalBoxes)
            {
                resultedBoudingBox.emplace_back(
                        cv::Point(std::abs(box[0][0]), std::abs(box[0][1])),
                        cv::Point(std::abs(box[2][0]), std::abs(box[2][1]))
                );
            }

            return resultedBoudingBox;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return {};
        }
    }
}