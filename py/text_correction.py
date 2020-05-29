from spellchecker import SpellChecker
import re


def paragraphize(text):
    text = text.replace('\n', ' ').replace('\r', '')
    cut_list = [0]
    result_texts = []

    ranges = [m.start() for m in re.finditer(r'\b(an)\b', text.lower())]

    result_texts.append(text[0:ranges[1]])
    for i in range(2, len(ranges)):
        result_texts.append(text[ranges[i-1]:ranges[i]])

    return result_texts


def correct_sentence(sentence):
    speller = SpellChecker(language='fr')

    res = ""
    for word in sentence.split():
        res += speller.correction(word) + " "

    return res
