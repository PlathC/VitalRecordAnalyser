from spellchecker import SpellChecker

def correct_sentence(sentence):
    speller = SpellChecker(language='fr')

    res = ""
    for word in sentence.split():
        res += speller.correction(word) + " "

    return res
