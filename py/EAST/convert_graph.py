import tensorflow.compat.v1 as tf

with tf.gfile.GFile('./frozen_graph.pb', 'rb') as f:
    graph_def = tf.GraphDef()
    graph_def.ParseFromString(f.read())

    for i in reversed(range(len(graph_def.node))):
        if graph_def.node[i].op == 'Const':
            del graph_def.node[i]

    tf.train.write_graph(graph_def, "", 'graph.pbtxt', as_text=True)
