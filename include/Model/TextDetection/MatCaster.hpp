//
// Created by Platholl on 15/05/2020.
//

#ifndef CIVILREGISTRYANALYSER_MATCASTER_HPP
#define CIVILREGISTRYANALYSER_MATCASTER_HPP

#include <stdexcept>

#include <opencv2/opencv.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

namespace pybind11 { namespace detail {

        template <> struct type_caster<cv::Mat> {
        public:
            /**
             * This macro establishes the name 'inty' in
             * function signatures and declares a local variable
             * 'value' of type inty
             */
            PYBIND11_TYPE_CASTER(cv::Mat, _("numpy.ndarray"));

            /**
             * Conversion part 1 (Python->C++): convert a PyObject into a inty
             * instance or return false upon failure. The second argument
             * indicates whether implicit conversions should be applied.
             */
            bool load(handle src, bool)
            {
                /* Try a default converting into a Python */
                array b(src, true);
                buffer_info info = b.request();

                std::size_t nDims = info.ndim;

                decltype(CV_32F) dType;
                if (info.format == format_descriptor<float>::format())
                {
                    if (nDims == 3)
                    {
                        dType = CV_32FC3;
                    }
                    else
                    {
                        dType = CV_32FC1;
                    }
                }
                else if (info.format == format_descriptor<double>::format())
                {
                    if (nDims == 3)
                    {
                        dType = CV_64FC3;
                    }
                    else
                    {
                        dType = CV_64FC1;
                    }
                }
                else if (info.format == format_descriptor<unsigned char>::format())
                {
                    if (nDims == 3)
                    {
                        dType = CV_8UC3;
                    }
                    else
                    {
                        dType = CV_8UC1;
                    }
                }
                else
                {
                    throw std::logic_error("Unsupported type");
                }

                std::vector<int> shape = { static_cast<int>(info.shape[0]),
                                           static_cast<int>(info.shape[1]) };

                value = cv::Mat(cv::Size(shape[1], shape[0]), dType, info.ptr, cv::Mat::AUTO_STEP);
                return true;
            }

            /**
             * Conversion part 2 (C++ -> Python): convert an inty instance into
             * a Python object. The second and third arguments are used to
             * indicate the return value policy and parent object (for
             * ``return_value_policy::reference_internal``) and are generally
             * ignored by implicit casters.
             */
            static handle cast(const cv::Mat &m, return_value_policy, handle defval)
            {
                std::string format = format_descriptor<unsigned char>::format();
                size_t elementSize;

                decltype(CV_64F) dim;
                switch(m.type()) {
                    case CV_8U:
                        format = format_descriptor<unsigned char>::format();
                        elementSize = sizeof(unsigned char);
                        dim = 2;
                        break;
                    case CV_8UC3:
                        format = format_descriptor<unsigned char>::format();
                        elementSize = sizeof(unsigned char);
                        dim = 3;
                        break;
                    case CV_32F:
                        format = format_descriptor<float>::format();
                        elementSize = sizeof(float);
                        dim = 2;
                        break;
                    case CV_64F:
                        format = format_descriptor<double>::format();
                        elementSize = sizeof(double);
                        dim = 2;
                        break;
                    default:
                        throw std::logic_error("Unsupported type");
                }

                std::vector<size_t> bufferDimension;
                std::vector<size_t> strides;
                if (dim == 2)
                {
                    bufferDimension = {(size_t) m.rows, (size_t) m.cols};
                    strides = {elementSize * (size_t) m.cols, elementSize};
                }
                else if (dim == 3)
                {
                    bufferDimension = {(size_t) m.rows, (size_t) m.cols, (size_t) 3};
                    strides = {(size_t) elementSize * m.cols * 3, (size_t) elementSize * 3, (size_t) elementSize};
                }

                return array(buffer_info(
                        m.data,         /* Pointer to buffer */
                        elementSize,       /* Size of one scalar */
                        format,         /* Python struct-style format descriptor */
                        dim,            /* Number of dimensions */
                        bufferDimension,      /* Buffer dimensions */
                        strides         /* Strides (in bytes) for each index */
                )).release();
            }

        };
    }} // namespace pybind11::detail


#endif //CIVILREGISTRYANALYSER_MATCASTER_HPP
