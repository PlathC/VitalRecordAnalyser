#define NOMINMAX
#include "tensorflow/cc/ops/standard_ops.h"
#include <tensorflow/core/framework/graph.pb.h>
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/protobuf/meta_graph.pb.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/framework/tensor.h"

using namespace tensorflow;

int main(int argc, char *argv[]) {
	// Create a Session running TensorFlow locally in process.
	std::unique_ptr<tensorflow::Session> session(tensorflow::NewSession({}));

	tensorflow::GraphDef graphDef;
    Status status;
	status = ReadBinaryProto(Env::Default(), "models/graph.pb", &graphDef);
    if (!status.ok()) {
        std::cout << status.ToString() << "\n";
        return 1;
    }
    // Add the graph to the session
    status = session->Create(graphDef);
    if (!status.ok()) {
        std::cout << status.ToString() << "\n";
        return 1;
    }

    // Setup inputs and outputs:

    // Our graph doesn't require any inputs, since it specifies default values,
    // but we'll change an input to demonstrate.
    Tensor a(DT_FLOAT, TensorShape());
    a.scalar<float>()() = 3.0;

    Tensor b(DT_FLOAT, TensorShape());
    b.scalar<float>()() = 2.0;

    std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
            { "a", a },
            { "b", b },
    };

    // The session will initialize the outputs
    std::vector<tensorflow::Tensor> outputs;

    // Run the session, evaluating our "c" operation from the graph
    status = session->Run(inputs, {"c"}, {}, &outputs);
    if (!status.ok()) {
        std::cout << status.ToString() << "\n";
        return 1;
    }

    // Grab the first output (we only evaluated one graph node: "c")
    // and convert the node to a scalar representation.
    auto output_c = outputs[0].scalar<float>();

    // (There are similar methods for vectors and matrices here:
    // https://github.com/tensorflow/tensorflow/blob/master/tensorflow/core/public/tensor.h)

    // Print the results
    std::cout << outputs[0].DebugString() << "\n"; // Tensor<type: float shape: [] values: 30>
    std::cout << output_c() << "\n"; // 30

    // Free any resources used by the session
    session->Close();
    return 0;
}
