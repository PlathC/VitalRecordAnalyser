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

spacy.prefer_gpu()

nlp = spacy.load("fr_core_news_sm")
doc = nlp(text)
for ent in doc.ents:
       print(ent.text, ent.start_char, ent.end_char, ent.label_)