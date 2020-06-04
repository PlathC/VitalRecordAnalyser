#coding: utf-8
"""
Provides options via the command line to perform project tasks.
* `--source`: dataset/model name (bentham, iam, rimes, saintgall, washington, civil)
* `--arch`: network to be used (puigcerver, bluche, flor)
* `--epochs`: number of epochs
* `--batch_size`: number of batches
"""

import argparse
import h5py
import os
import string

from data.reader import Dataset

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=str, default="civil")
    parser.add_argument("--arch", type=str, default="flor")

    parser.add_argument("--epochs", type=int, default=1000)
    parser.add_argument("--batch_size", type=int, default=16)
    args = parser.parse_args()

    raw_path = os.path.join("..", "raw", args.source)
    source_path = os.path.join("..", "data", f"{args.source}.hdf5")

    input_size = (1024, 128, 1)
    max_text_length = 128
    charset_base = string.printable[:95]

    assert os.path.exists(raw_path)
    print(f"The {args.source} dataset will be transformed...")

    ds = Dataset(source=raw_path, name=args.source)
    ds.read_partitions()

    print("Partitions will be preprocessed...")
    ds.preprocess_partitions(input_size=input_size)

    print("Partitions will be saved...")
    os.makedirs(os.path.dirname(source_path), exist_ok=True)

    for i in ds.partitions:
        with h5py.File(source_path, "a") as hf:
            hf.create_dataset(f"{i}/dt", data=ds.dataset[i]['dt'], compression="gzip", compression_opts=9)
            hf.create_dataset(f"{i}/gt", data=ds.dataset[i]['gt'], compression="gzip", compression_opts=9)
            print(f"[OK] {i} partition.")

    print(f"Transformation finished.")