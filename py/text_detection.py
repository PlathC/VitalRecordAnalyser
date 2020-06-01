import argparse
import cv2
import h5py
import os
import string
import datetime

from numba import cuda

import tensorflow as tf

from data import preproc as pp, evaluation
from data.generator import DataGenerator, Tokenizer
from data.reader import Dataset
from kaldiio import WriteHelper
from network.model import HTRModel

tokenizer = None
model = None
input_size = (1024, 128, 1)
max_text_length = 128
charset_base = string.printable[:95]

gpus = tf.config.experimental.list_physical_devices('GPU')
if gpus:
    # Restrict TensorFlow to only allocate 1GB of memory on the first GPU
    try:
        tf.config.experimental.set_virtual_device_configuration(
            gpus[0],
            [tf.config.experimental.VirtualDeviceConfiguration(memory_limit=1024)])
        logical_gpus = tf.config.experimental.list_logical_devices('GPU')
        print(len(gpus), "Physical GPUs,", len(logical_gpus), "Logical GPUs")
    except RuntimeError as e:
        # Virtual devices must be set before GPUs have been initialized
        print(e)


def read_all_text_from_images(imgs):
    output = []
    for img in imgs:
        output.append(read_text_from_image(img))

    return output


def load_model():
    global model, tokenizer, charset_base, max_text_length
    tokenizer = Tokenizer(chars=charset_base, max_text_length=max_text_length)
    model = HTRModel(architecture="flor",
                     input_size=input_size,
                     vocab_size=tokenizer.vocab_size,
                     top_paths=10)
    model.compile()
    model.load_checkpoint(target="./py/checkpoint_weights.hdf5")


def read_text_from_image(img):
    global model, tokenizer, input_size

    img = pp.preprocess(img, input_size=input_size)
    x_test = pp.normalization([img])

    predicts, probabilities = model.predict(x_test, ctc_decode=True)
    predicts = [[tokenizer.decode(x) for x in y] for y in predicts]

    for i, (pred, prob) in enumerate(zip(predicts, probabilities)):
        return pred[0]
