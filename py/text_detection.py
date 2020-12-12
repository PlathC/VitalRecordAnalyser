import argparse
import cv2
import h5py
import os
import string
import datetime
import sys
import logging

from numba import cuda

import tensorflow as tf

from data import preproc as pp, evaluation
from data.generator import DataGenerator, Tokenizer
from data.reader import Dataset
from kaldiio import WriteHelper
from network.model import HTRModel
import ml_utils


log = logging.getLogger('tensorflow')
log.setLevel(logging.DEBUG)

handler = logging.StreamHandler(sys.stdout)
handler.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)
log.addHandler(handler)


def resize_with_aspect_ratio(image, width=None, height=None, inter=cv2.INTER_AREA):
    dim = None
    (h, w) = image.shape[:2]

    if width is None and height is None:
        return image

    if width is None:
        r = height / float(h)
        dim = (int(w * r), height)
    else:
        r = width / float(w)
        dim = (width, int(h * r))

    return cv2.resize(image, dim, interpolation=inter)


def show_image(img):
    img = resize_with_aspect_ratio(img, width=800)
    cv2.imshow('resize', img)
    cv2.waitKey(0)


class TextRecognizer:

    def __init__(self, checkpoint_path="./py/checkpoint_weights.hdf5", input_size=(1024, 128, 1), max_text_length=128, charset_base=string.printable[:95], architecture="flor"):
        self.tokenizer = None
        self.model = None
        self.checkpoint_path = checkpoint_path
        self.input_size = input_size
        self.max_text_length = max_text_length
        self.charset_base = charset_base
        self.architecture = architecture
        ml_utils.limit_gpu_memory()

        self.load_model()

    def load_model(self):
        self.tokenizer = Tokenizer(chars=self.charset_base, max_text_length=self.max_text_length)
        self.model = HTRModel(architecture=self.architecture,
                              input_size=self.input_size,
                              vocab_size=self.tokenizer.vocab_size,
                              top_paths=10)
        self.model.compile()
        self.model.load_checkpoint(target=self.checkpoint_path)

    def read_all_text_from_images(self, images):
        output = []
        for img in images:
            output.append(self.read_text_from_image(img))

        return output

    def read_text_from_image(self, img):
        img = pp.preprocess(img, input_size=self.input_size)
        x_test = pp.normalization([img])

        predicts, probabilities = self.model.predict(x_test, ctc_decode=True)
        predicts = [[self.tokenizer.decode(x) for x in y] for y in predicts]

        for i, (pred, prob) in enumerate(zip(predicts, probabilities)):
            return pred[0]

        return ""
