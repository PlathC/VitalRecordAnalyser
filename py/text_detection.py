import argparse
import cv2
import h5py
import os
import string
import datetime

import tensorflow as tf

from data import preproc as pp, evaluation
from data.generator import DataGenerator, Tokenizer
from data.reader import Dataset
from kaldiio import WriteHelper
from network.model import HTRModel


def read_all_text_from_images(imgs):
    output = []
    for img in imgs:
        output.append(read_text_from_image(img))

    return output


def read_text_from_image(img):
    input_size = (1024, 128, 1)
    max_text_length = 128
    charset_base = string.printable[:95]
    tokenizer = Tokenizer(chars=charset_base, max_text_length=max_text_length)
    img = pp.preprocess(img, input_size=input_size)
    x_test = pp.normalization([img])

    model = HTRModel(architecture="flor",
                     input_size=input_size,
                     vocab_size=tokenizer.vocab_size,
                     top_paths=10)

    model.compile()

    model.load_checkpoint(target="./py/checkpoint_weights.hdf5")

    predicts, probabilities = model.predict(x_test, ctc_decode=True)
    predicts = [[tokenizer.decode(x) for x in y] for y in predicts]

    # max_pred = 0
    # associated_pred = ""
    for i, (pred, prob) in enumerate(zip(predicts, probabilities)):
        return pred[0]
    #     for (pd, pb) in zip(pred, prob):
    #         if pb > max_pred:
    #             max_pred = pb
    #             associated_pred = pd
    # return associated_pred
