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
    target_path = os.path.join(output_path, "checkpoint_recognizer.hdf5")

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

    if args.train:
        model.summary(output_path, "summary.txt")
        callbacks = model.get_callbacks(logdir=output_path, checkpoint=target_path, verbose=1)

        start_time = datetime.datetime.now()

        h = model.fit(x=dtgen.next_train_batch(),
                      epochs=args.epochs,
                      steps_per_epoch=dtgen.steps['train'],
                      validation_data=dtgen.next_valid_batch(),
                      validation_steps=dtgen.steps['valid'],
                      callbacks=callbacks,
                      shuffle=True,
                      verbose=1)

        total_time = datetime.datetime.now() - start_time

        loss = h.history['loss']
        val_loss = h.history['val_loss']

        min_val_loss = min(val_loss)
        min_val_loss_i = val_loss.index(min_val_loss)

        time_epoch = (total_time / len(loss))
        total_item = (dtgen.size['train'] + dtgen.size['valid'])

        t_corpus = "\n".join([
            f"Total train images:      {dtgen.size['train']}",
            f"Total validation images: {dtgen.size['valid']}",
            f"Batch:                   {dtgen.batch_size}\n",
            f"Total time:              {total_time}",
            f"Time per epoch:          {time_epoch}",
            f"Time per item:           {time_epoch / total_item}\n",
            f"Total epochs:            {len(loss)}",
            f"Best epoch               {min_val_loss_i + 1}\n",
            f"Training loss:           {loss[min_val_loss_i]:.8f}",
            f"Validation loss:         {min_val_loss:.8f}"
        ])

        with open(os.path.join(output_path, "train.txt"), "w") as lg:
            lg.write(t_corpus)
            print(t_corpus)