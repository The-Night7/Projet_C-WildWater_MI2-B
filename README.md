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
dos2unix scripts/myScript.sh
./scripts/myScript.sh histo max    # Capacité maximale
./scripts/myScript.sh histo src    # Volume sources
./scripts/myScript.sh histo real   # Volume réel traité
./scripts/myScript.sh histo all    # Bonus : Graphique unifiant les trois modes
```
**Exemple leaks**
```bash
./scripts/myScript.sh leaks "Facility complex #RH400057F"
```
