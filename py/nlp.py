import spacy

text = "L ' An mil neuf cent un , le Quatres Janvier de feuilles ah a cinq heures du " \
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


def is_token_allowed(token):
    '''
             Only allow valid tokens which are not stop words
             and punctuation symbols.
    '''
    if (not token or not token.string.strip() or
            token.is_stop or token.is_punct):
        return False
    return True


def preprocess_token(token):
    # Reduce token to its lowercase lemma form
    return token.lemma_.strip().lower()


spacy.prefer_gpu()

nlp = spacy.load("fr_core_news_sm")

# nlp.add_pipe(set_custom_boundaries, before='parser')
doc = nlp(text)
complete_filtered_tokens = [preprocess_token(token) for token in doc if is_token_allowed(token)]

dic = {}

curr_ent = doc.ents[0]
ite = 1
dic["Naissance"] = ''
while len(doc.ents) > ite and curr_ent.label_ == 'MISC':
    dic["Naissance"] += curr_ent.text + ' '
    ite += 1
    curr_ent = doc.ents[ite]

while len(doc.ents) > ite and curr_ent.label_ != 'PER':
    ite += 1
    curr_ent = doc.ents[ite]

print(len(doc.ents) > ite)

if len(doc.ents) > ite:
    dic["Charles"] = curr_ent.text
    ite += 1

while len(doc.ents) > ite and curr_ent.label_ != 'LOC':
    ite += 1
    curr_ent = doc.ents[ite]

dic["LieuNaissance"] = ""
while len(doc.ents) > ite and curr_ent.label_ == 'LOC':
    dic["LieuNaissance"] += curr_ent.text + ' '
    ite += 1
    curr_ent = doc.ents[ite]

while len(doc.ents) > ite and curr_ent.label_ != 'PER':
    ite += 1
    curr_ent = doc.ents[ite]

if len(doc.ents) > ite:
    dic["Déclarant"] = curr_ent.text
    ite += 1
    curr_ent = doc.ents[ite]

while len(doc.ents) > ite and curr_ent.label_ != 'PER':
    ite += 1
    curr_ent = doc.ents[ite]

if len(doc.ents) > ite:
    dic["Mère"] = curr_ent.text
    ite += 1
    curr_ent = doc.ents[ite]

while len(doc.ents) > ite and curr_ent.label_ != 'PER':
    ite += 1
    curr_ent = doc.ents[ite]

if len(doc.ents) > ite:
    dic["NomEnfant"] = curr_ent.text
    ite += 1
    curr_ent = doc.ents[ite]

while len(doc.ents) > ite and curr_ent.label_ != 'PER':
    ite += 1
    curr_ent = doc.ents[ite]

if len(doc.ents) > ite:
    dic["Témoins1"] = curr_ent.text
    ite += 1
    curr_ent = doc.ents[ite]

while len(doc.ents) > ite and curr_ent.label_ != 'PER':
    ite += 1
    curr_ent = doc.ents[ite]

if len(doc.ents) > ite:
    dic["Témoins2"] = curr_ent.text
    ite += 1

print(dic)
