# C-WildWater - Guide d'utilisation

Ce document fournit les instructions détaillées pour compiler et utiliser le système d'analyse de données C-WildWater.

## Prérequis

- Compilateur GCC (version 5.0 ou supérieure recommandée)
- GNU Make
- GnuPlot (pour la génération des graphiques)
- Bash (pour les scripts Shell)

## Compilation

Pour compiler le programme, suivez ces étapes :

1. Ouvrez un terminal et naviguez vers le répertoire `src` du projet :
   ```
   cd chemin/vers/C-WildWater/src
   ```

2. Exécutez la commande de compilation avec Make :
   ```
   make
   ```

3. Vérifiez que le programme a bien été compilé :
   ```
   ls -l c-wildwater
   ```

## Utilisation du script principal

Le script principal `myScript.sh` se trouve dans le répertoire `scripts`. Il sert de point d'entrée pour toutes les fonctionnalités du projet.

### Syntaxe générale

```
./scripts/myScript.sh <commande> [options]
```

### Commandes disponibles

1. **Génération d'histogrammes** :
   ```
   ./scripts/myScript.sh histo max    # Capacité maximale des usines
   ./scripts/myScript.sh histo src    # Volume d'eau capté
   ./scripts/myScript.sh histo real   # Volume d'eau réellement traité
   ./scripts/myScript.sh histo all    # Histogramme cumulé (bonus)
   ```

2. **Calcul des pertes d'eau** :
   ```
   ./scripts/myScript.sh leaks <identifiant_usine>   # Ex: ./scripts/myScript.sh leaks USINE123
   ```

### Exemples

- Pour générer un histogramme des capacités maximales des usines :
  ```
  ./scripts/myScript.sh histo max
  ```

- Pour calculer les pertes d'eau pour une usine spécifique :
  ```
  ./scripts/myScript.sh leaks USINE456
  ```

## Résultats générés

Les résultats générés sont stockés dans le répertoire `data` :

- `vol_max.dat` : Résultats de l'histogramme de capacité maximale
- `vol_captation.txt` : Résultats de l'histogramme de volume capté
- `vol_traitement.tmp` : Résultats de l'histogramme de volume traité
- `leaks_history.dat` : Historique des pertes d'eau par usine
- `output_images/` : Contient les graphiques générés en format PNG

## Exécution des tests

Pour exécuter les tests automatisés :

1. Assurez-vous d'abord que le programme est compilé :
   ```
   cd src && make
   ```

2. Exécutez les tests :
   ```
   cd ../tests && ./run_tests.sh
   ```

## Format des fichiers de données

Les données d'entrée sont fournies dans deux fichiers au format .dat :
- `c-wildwater_v0.dat` : Contient les données de base du réseau d'eau
- `c-wildwater_v3.dat` : Contient les données complémentaires pour l'analyse

## Dépannage

Si vous rencontrez des problèmes :

1. Vérifiez que toutes les dépendances sont installées
2. Assurez-vous que les fichiers de données sont présents dans le répertoire `data`
3. Vérifiez les permissions d'exécution des scripts (sous Linux/Mac) :
   ```
   chmod +x scripts/myScript.sh tests/run_tests.sh
   ```
4. Consultez les messages d'erreur qui fournissent souvent des indications précises sur le problème