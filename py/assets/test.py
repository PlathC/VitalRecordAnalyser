#coding: utf-8
"""
Provides options via the command line to perform project tasks.
* `--source`: dataset/model name (bentham, iam, rimes, saintgall, washington, civil)
* `--arch`: network to be used (puigcerver, bluche, flor)
* `--epochs`: number of epochs
* `--batch_size`: number of batches
"""

import argparse
import os
import string
import datetime

from data import evaluation
from data.generator import DataGenerator, Tokenizer
from network.model import HTRModel

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=str, default="civil")
    parser.add_argument("--arch", type=str, default="flor")

    parser.add_argument("--epochs", type=int, default=1000)
    parser.add_argument("--batch_size", type=int, default=16)
    args = parser.parse_args()

    source_path = os.path.join("..", "data", f"{args.source}.hdf5")
    output_path = os.path.join("..", "output", args.source, args.arch)
    target_path = os.path.join(output_path, "checkpoint_weights.hdf5")

    input_size = (1024, 128, 1)
    max_text_length = 128
    charset_base = string.printable[:95]

    assert os.path.isfile(source_path) or os.path.isfile(target_path)
    os.makedirs(output_path, exist_ok=True)

    dtgen = DataGenerator(source=source_path,
                          batch_size=args.batch_size,
                          charset=charset_base,
                          max_text_length=max_text_length,
                          predict=args.test)

    model = HTRModel(architecture=args.arch,
                     input_size=input_size,
                     vocab_size=dtgen.tokenizer.vocab_size)

    # set `learning_rate` parameter or get architecture default value
    model.compile(learning_rate=0.001)
    model.load_checkpoint(target=target_path)

    start_time = datetime.datetime.now()

    predicts, _ = model.predict(x=dtgen.next_test_batch(),
                                steps=dtgen.steps['test'],
                                ctc_decode=True,
                                verbose=1)

    predicts = [dtgen.tokenizer.decode(x[0]) for x in predicts]

    total_time = datetime.datetime.now() - start_time

    with open(os.path.join(output_path, "predict.txt"), "w") as lg:
        for pd, gt in zip(predicts, dtgen.dataset['test']['gt']):
            lg.write(f"TE_L {gt}\nTE_P {pd}\n")

    for i, item in enumerate(dtgen.dataset['test']['dt'][:10]):
        print("=" * 1024, "\n")
        print(dtgen.dataset['test']['gt'][i])
        print(predicts[i], "\n")

    evaluate = evaluation.ocr_metrics(predicts=predicts,
                                      ground_truth=dtgen.dataset['test']['gt'],
                                      norm_accentuation=False,
                                      norm_punctuation=False)

    e_corpus = "\n".join([
        f"Total test images:    {dtgen.size['test']}",
        f"Total time:           {total_time}",
        f"Time per item:        {total_time / dtgen.size['test']}\n",
        f"Metrics:",
        f"Character Error Rate: {evaluate[0]:.8f}",
        f"Word Error Rate:      {evaluate[1]:.8f}",
        f"Sequence Error Rate:  {evaluate[2]:.8f}"
    ])

    with open(os.path.join(output_path, "evaluate.txt"), "w") as lg:
        lg.write(e_corpus)
        print(e_corpus)