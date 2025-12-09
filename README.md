# C‑WildWater

Ce dépôt contient la solution fonctionnelle pour le projet **C‑WildWater**.  Le
but du projet est d’analyser de grandes quantités de données décrivant un
réseau d’eau potable et d’extraire deux types d’informations :

* **Histogrammes** : agréger les capacités de traitement, les volumes captés
  et les volumes réellement traités pour chaque usine.  Les histogrammes
  permettent d’identifier les installations qui traitent le plus ou le moins de
  volume.
* **Pertes** : calculer le volume d’eau perdu dans le réseau en aval d’une
  usine donnée à partir des fuites indiquées sur chaque tronçon.

L’arborescence du projet respecte la structure recommandée dans le sujet :

```
C‑WildWater/
├── src/                # Code source en C (programme principal et bibliothèques)
│   ├── main.c          # Point d’entrée du programme (lecture et calculs)
│   ├── avl.c           # Implémentation de l’arbre AVL adapté au problème
│   ├── avl.h           # Prototype des fonctions AVL
│   ├── structs.h       # Structures utilisées (Station et AdjNode)
│   ├── utils.c         # Fonctions utilitaires génériques (facultatif)
│   ├── utils.h         # En‑têtes des fonctions utilitaires
│   └── Makefile        # Recette de compilation du binaire « c‑wildwater »
│
├── scripts/            # Scripts shell d’orchestration
│   ├── myScript.sh     # Script principal pour générer des histogrammes et calculer les fuites
│   └── vags.bash       # Exemple de script de gestion/compilation fourni
│
├── data/               # Données d’entrée et de sortie
│   ├── (vide)          # Déposez vos fichiers .dat/.csv ici
│   └── output_images/  # Les images générées par GnuPlot seront créées ici
│
├── doc/                # Documentation fournie
│   └── Projet_C‑WildWater_preIng2_2025_2026_v1.1.pdf
│
├── .gitignore          # Fichiers à ignorer par Git (optionnel)
└── LICENSE             # Licence du projet (optionnel)
```

## Compilation

Le binaire principal s’appelle **c‑wildwater** et se compile avec :

```sh
cd src
make
```

Cela génère un exécutable `c‑wildwater` dans le dossier `src/`.  La
commande `make clean` supprime les fichiers objets et l’exécutable.

## Utilisation rapide

Deux modes de fonctionnement sont supportés par le programme :

* **Mode histogramme** : calcule et exporte les volumes par usine.  Trois
  modes sont possibles : `max` (capacité maximale), `src` (volume capté)
  et `real` (volume réellement traité après fuites).  Exemple :

  ```sh
  ./c‑wildwater chemin/vers/fichier.dat max > vol_max.csv
  ```

  Le fichier CSV généré contient deux colonnes séparées par un point‑virgule :
  l’identifiant de l’usine et la valeur agrégée (en milliers de mètres cubes).

* **Mode fuites** : calcule la quantité totale d’eau perdue (en millions de
  mètres cubes) dans le réseau en aval d’une usine donnée.  Exemple :

  ```sh
  ./c‑wildwater chemin/vers/fichier.dat Facility\ complex\ #RH400057F
  ```

  Le programme affiche le volume d’eau perdu à partir de la capacité de
  l’usine indiquée.  Si l’usine n’est pas trouvée, `0` est affiché.

Pour automatiser l’exécution et la génération de graphiques, vous pouvez
utiliser le script `scripts/myScript.sh`.  Ce dernier compile le projet si
nécessaire, lance le programme dans le mode demandé et utilise `gnuplot`
pour générer des histogrammes « Top 10 » et « Bottom 50 ».  Les images sont
placées dans `data/output_images/`.

## Remarques

* Les modules `csv_io.c`, `histogram.c` et `make_csv.c` fournis initialement
  ont été conservés à titre documentaire mais ne sont pas nécessaires pour la
  solution finale.  Le programme principal `main.c` effectue directement le
  filtrage et l’agrégation à partir du fichier d’entrée.
* Pensez à installer `gnuplot` pour générer les graphiques si ce n’est pas
  déjà le cas dans votre environnement.
* Ce projet constitue une base de travail.  N’hésitez pas à étoffer la
  documentation, ajouter des tests automatisés dans un dossier `tests/` et
  améliorer les scripts selon vos besoins.
