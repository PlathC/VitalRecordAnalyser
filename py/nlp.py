import spacy


def wait_for_and_add(document, curr_ite, dictionary, list_key, label):
    if len(document.ents) > curr_ite:
        curr_entity = document.ents[curr_ite]
        while len(document.ents) > curr_ite and curr_entity.label_ != label:
            curr_entity = document.ents[curr_ite]
            curr_ite += 1

        if len(document.ents) > curr_ite:
            dictionary[list_key] = curr_entity.text
            curr_ite += 1

    return curr_ite, dictionary


sample_text = "L ' An mil neuf cent un , le Quatres Janvier de feuilles ah a cinq heures du " \
              "soir pardevant nous Charles Schneider Maire de la Vile de Belfort ( Haut - Rhin ) ." \
              " officier de l ' etat - civil de la dite Vile , est comparu a Mr l ' Hotel - de - Vile" \
              " le sieur Joseph meyer , age de vingt sept ans forgeron fier cirie en cete vile faubourg " \
              "des gosses , cent cinquante lequel nous a pre me berger sente un enfant du sexe féminin ," \
              " ne aujourd ' hui , quatre Janvier , a Marie antoinette huit heures un quart du matin ," \
              " en sa demeure , de lui déclarant , et rue madeleine river , age de vingt sept ans ," \
              " sans profession are B al il maison epouse , aussi domicile a Belfort , et auquel il " \
              "a déclare vouloir 1921 donner les prenons de Marie antoinette . les dites déclaration et" \
              " ar9releleAe tel ainsi on p faites en presence des soeurs Arthur tels , age de quarante" \
              " deux nor hippolyte aimer , age de cinquante six ans , les deux forgerons domiciles a " \
              "Belfort decide jesus HeH ( on ) , el 1 Le 6 Juin 198 Et ont les et temoins , signe avec" \
              " nous le present acte , apres lecture . boulder npr parie "

nlp = None


def init_nlp_module():
    global nlp
    nlp = spacy.load("fr_core_news_sm")


def process_text(text):
    global nlp
    doc = nlp(text)
    if len(doc.ents) != 0:
        dic = {}

        curr_ent = doc.ents[0]
        ite = 1

        ite, dic = wait_for_and_add(doc, ite, dic, 'Naissance', 'MISC')
        ite, dic = wait_for_and_add(doc, ite, dic, 'LieuNaissance', 'LOC')
        ite, dic = wait_for_and_add(doc, ite, dic, 'Déclarant', 'PER')
        ite, dic = wait_for_and_add(doc, ite, dic, 'Mère', 'PER')
        ite, dic = wait_for_and_add(doc, ite, dic, 'NomEnfant', 'PER')
        ite, dic = wait_for_and_add(doc, ite, dic, 'Témoins1', 'PER')
        ite, dic = wait_for_and_add(doc, ite, dic, 'Témoins2', 'PER')

        return dic
    else:
        return {}