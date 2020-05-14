// main.cpp
#define FDEEP_FLOAT_TYPE double

#include <algorithm>

#include <fdeep/fdeep.hpp>
#include <opencv2/opencv.hpp>

namespace fdeep {
    namespace internal {
        class FullGatedConv2D : public conv_2d_layer {
        public:
            explicit FullGatedConv2D(const std::string &name, const size_t &nb_filters,
                                     const tensor_shape &filter_shape,
                                     std::size_t k, const shape2 &strides, padding p,
                                     const shape2 &dilation_rate,
                                     const float_vec &weights, const float_vec &bias)
                    : conv_2d_layer(name, filter_shape, k, strides,
                                    p, dilation_rate, weights, bias),
                      m_nbFilters(nb_filters)
            {

            }

        protected:
            tensors apply_impl(const tensors &input) const override
            {
                tensors temp = conv_2d_layer::apply_impl(input);
                auto linear = std::make_shared<linear_layer>("linear");
                auto sigmoid = std::make_shared<sigmoid_layer>("sigmoid");
                const auto &tempInput = single_tensor_from_tensors(temp);

                tensor_shape nShape(tempInput.shape().height_, tempInput.shape().width_, tempInput.shape().depth_/2);

                tensor sigmoidOutput(nShape, 0);
                tensor linearOutput(nShape, 0);
                for (std::size_t dim5 = 0; dim5 < tempInput.shape().size_dim_5_; ++dim5)
                {
                    for (std::size_t dim4 = 0; dim4 < tempInput.shape().size_dim_4_; ++dim4)
                    {
                        for (std::size_t x = 0; x < tempInput.shape().height_; ++x)
                        {
                            for (std::size_t y = 0; y < tempInput.shape().width_; ++y)
                            {
                                for (std::size_t z = 0; z < tempInput.shape().depth_; ++z)
                                {
                                    float_type val = tempInput.get(tensor_pos(y, x, z));

                                    if (z > tempInput.shape().depth_ / 2 - 1)
                                    {
                                        sigmoidOutput.set(tensor_pos(y, x, z), val);
                                    }
                                    else
                                    {
                                        linearOutput.set(tensor_pos(y, x, z), val);
                                    }
                                }
                            }
                        }
                    }
                }

                auto linearValue = apply_activation_layer(linear, {linearOutput});
                auto sigmoidValue = apply_activation_layer(sigmoid, {sigmoidOutput});
                std::vector<tensor> toMultiply;
                toMultiply.insert(toMultiply.end(), linearValue.begin(), linearValue.end());
                toMultiply.insert(toMultiply.end(), sigmoidValue.begin(), sigmoidValue.end());

                return {multiply_tensors(toMultiply)};
            }

            size_t m_nbFilters;
        };


        inline layer_ptr create_FullGatedConv2D(const get_param_f &get_param,
                                                const nlohmann::json &data,
                                                const std::string &name) {
            const std::string padding_str = data["config"]["padding"];
            const auto pad_type = create_padding(padding_str);

            const shape2 strides = create_shape2(data["config"]["strides"]);
            const shape2 dilation_rate = create_shape2(data["config"]["dilation_rate"]);

            const auto nb_filters = create_size_t(data["config"]["nb_filters"]);
            const auto filter_count = nb_filters * 2;
            float_vec bias(filter_count, 0);
            const bool use_bias = data["config"]["use_bias"];
            if (use_bias)
                bias = decode_floats(get_param(name, "bias"));
            assertion(bias.size() == filter_count, "size of bias does not match");

            const float_vec weights = decode_floats(get_param(name, "weights"));
            const shape2 kernel_size = create_shape2(data["config"]["kernel_size"]);
            assertion(weights.size() % kernel_size.area() == 0,
                      "invalid number of weights");
            const std::size_t filter_depths =
                    weights.size() / (kernel_size.area() * filter_count);
            const tensor_shape filter_shape(
                    kernel_size.height_, kernel_size.width_, filter_depths);

            return std::make_shared<FullGatedConv2D>(name, nb_filters, filter_shape, filter_count, strides, pad_type,
                                                     dilation_rate, weights, bias);
        }
    }
}

int main() {
    const cv::Mat image = cv::imread("Germaine.png");
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_RGB2GRAY);
    const double WishingWidth  = 1024.;
    const double WishingHeight = 128.;
    const double CurrentWidth  = static_cast<double>(gray.size().width);
    const double CurrentHeight = static_cast<double>(gray.size().height);

    double f = std::max((CurrentWidth / WishingWidth),
                          (CurrentHeight / WishingHeight));

    double divW = std::floor(CurrentWidth / f);
    double minWidth = std::min(WishingWidth, divW);
    double newWidth = std::max(minWidth, 1.);

    double divH = static_cast<double>(std::floor(CurrentHeight / f));
    double minHeight = std::min(WishingHeight, std::floor(CurrentHeight / f));
    double newHeight = std::max(minHeight, 1.);
    cv::resize(gray, gray, cv::Size(1024, 128));
    assert(image.isContinuous());

    try {
        auto creators = fdeep::internal::layer_creators{
                {"FullGatedConv2D", fdeep::internal::create_FullGatedConv2D}
        };

        const auto model =
                fdeep::load_model("./modelunopti.json", true,
                                  fdeep::cout_logger, static_cast<fdeep::float_type>(1),
                                  creators);

        const auto input = fdeep::tensor_from_bytes(gray.ptr(),
                                                    static_cast<std::size_t>(gray.cols),
                                                    static_cast<std::size_t>(gray.rows),
                                                    static_cast<std::size_t>(gray.channels()),
                                                    0, 1);
        try {
            const auto result = model.predict({input});
            std::cout << fdeep::show_tensors(result) << std::endl;

        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error while loading : " << e.what() << std::endl;
        return -1;
    }
}