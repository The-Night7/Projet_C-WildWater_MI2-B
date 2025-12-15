<<<<<<< HEAD
# C‑WildWater

<<<<<<< HEAD
## Présentation
C-WildWater est un projet pour analyser et visualiser les données liées à la gestion de l'eau.

## Liens utiles
- [Instructions](doc/ReadMe.md)
- [Architecture](doc/architecture.md)
- [CheckList](doc/guide_de_travail_projet.md)
=======
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
>>>>>>> origin/teuteu_test
=======
# 🌊 C-WildWater : Analyse de Réseau Hydraulique

> **Traitement massif de données & Algorithmique en C**

![Language](https://img.shields.io/badge/Language-C-blue) ![Script](https://img.shields.io/badge/Script-Bash-green) ![Build](https://img.shields.io/badge/Build-Make-orange)

## 📖 À propos du projet

**C-WildWater** est une application haute performance conçue pour analyser un réseau de distribution d'eau potable simulant **1/3 du réseau français**.

Face à un fichier de données massif (plusieurs millions de lignes, >500Mo), ce projet combine la flexibilité du **Shell** et la puissance du **C** pour :
1.  Ingérer et structurer les données (Graphes & Arbres AVL).
2.  Générer des statistiques précises sur les usines de traitement.
3.  Détecter les fuites et calculer les pertes sur l'ensemble du réseau.
4.  Visualiser les résultats via des graphiques dynamiques.

---

## 🚀 Fonctionnalités Clés

### 📊 1. Analyse des Volumes (Mode Histo)
Génération de fichiers CSV et de graphiques via **Gnuplot** pour visualiser :
* **Capacité :** Le volume maximal que les usines peuvent traiter.
* **Captage :** Le volume d'eau réellement puisé aux sources.
* **Réel :** Le volume final distribué (après fuites).
* ✨ **BONUS :** Un histogramme cumulé ("All") visualisant les 3 états simultanément (Capacité / Pertes / Sortie).

### 💧 2. Calcul de Fuites (Mode Leaks)
Un algorithme de parcours de graphe (DFS) optimisé pour calculer le volume total d'eau perdu en aval d'une usine spécifique.
* **Performance :** Temps de traitement optimisé (millisecondes).
* **Précision :** Prise en compte des pourcentages de fuite à chaque tronçon.
* ✨ **BONUS :** Identification automatique du tronçon critique (pire fuite en valeur absolue).

---

## 🛠️ Installation & Prérequis

Ce projet est conçu pour fonctionner sous un environnement **Linux** (ou WSL).

**Dépendances nécessaires :**
```bash
sudo apt update
sudo apt install build-essential gnuplot make
```
## Utilisation :
```bash
./scripts/myScript.sh histo max    # Capacité maximale
./scripts/myScript.sh histo src    # Volume sources
./scripts/myScript.sh histo real   # Volume réel traité
./scripts/myScript.sh histo all
```
**Exemple leaks**
```bash
./scripts/myScript.sh leaks "Facility complex #RH400057F"
```
>>>>>>> origin/main
