# Architecture du projet C-WildWater

Ce document décrit l'architecture technique du projet C-WildWater, incluant la structure des fichiers, les modules principaux et leurs interactions.

## Structure générale

Le projet est organisé selon une architecture modulaire, avec une séparation claire entre :
- Le code source en C pour les traitements intensifs
- Les scripts Shell pour l'orchestration
- Les données d'entrée et de sortie
- Les tests automatisés

## Modules principaux

### Module Shell (scripts/myScript.sh)

Ce script sert de point d'entrée au programme et:
- Analyse les arguments fournis par l'utilisateur
- Vérifie la présence des fichiers nécessaires
- Appelle le programme C avec les bons paramètres
- Affiche les messages appropriés à l'utilisateur
- Mesure le temps d'exécution

### Modules C

#### Module principal (main.c)

- Point d'entrée du programme C
- Analyse les arguments et appelle les fonctions appropriées
- Gère les codes d'erreur et les messages

#### Module histogramme (histogram.c/h)

- Génère les différents types d'histogrammes demandés
- Utilise une structure AVL pour stocker et trier les données des usines
- Écrit les résultats dans les fichiers appropriés
- Génère les images PNG via GnuPlot

#### Module pertes (leaks.c/h)

- Calcule les pertes d'eau dans le réseau aval d'une usine
- Modélise le réseau sous forme d'arbre
- Implémente les algorithmes de parcours d'arbre
- Gère l'historique des pertes

#### Module utilitaire (utils.c/h)

- Fournit des fonctions génériques utilisées par les autres modules
- Implémente la structure de données AVL
- Gère la lecture des fichiers de données
- Offre des fonctions de manipulation de chaînes et de conversion

## Structures de données

### Arbre AVL

Utilisé pour stocker les identifiants d'usines et optimiser les recherches :
- Chaque nœud contient un identifiant d'usine (clé) et des données associées
- Structure équilibrée garantissant des recherches en O(log n)
- Implémentation complète avec rotations et rééquilibrage

### Arbre de distribution

Utilisé pour modéliser le réseau de distribution d'eau :
- Chaque nœud représente un élément du réseau (source, usine, jonction, etc.)
- Les arêtes représentent les connexions entre les éléments
- Permet le calcul récursif des pertes d'eau

## Flux de données

1. Les données sont lues depuis les fichiers d'entrée (format DAT)
2. Elles sont chargées dans les structures de données appropriées
3. Les calculs sont effectués selon le mode demandé
4. Les résultats sont écrits dans les fichiers de sortie
5. Les graphiques sont générés à partir des résultats

## Gestion des erreurs

- Vérification systématique des allocations de mémoire
- Validation des fichiers d'entrée et des paramètres
- Codes de retour spécifiques selon le type d'erreur
- Messages d'erreur explicites pour faciliter le débogage

## Extensions possibles

- Parallélisation des calculs pour améliorer les performances
- Ajout d'une interface graphique pour visualiser les résultats
- Implémentation d'algorithmes plus sophistiqués pour l'analyse des pertes