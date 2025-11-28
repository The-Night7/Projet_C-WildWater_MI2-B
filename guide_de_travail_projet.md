# Etapes de Travail pour le Projet d'Info

## 1. Analyse préliminaire et conception

### 1.1 Compréhension des données
- [ ] Examiner la structure du fichier CSV (plus de 8 millions de lignes, 500 Mo)
- [ ] Identifier les colonnes clés et leur signification
- [ ] Comprendre la topologie du réseau (sources → usines → stockage → distribution → usagers)

### 1.2 Conception de l'architecture
- [ ] Définir les structures de données nécessaires:
  - AVL pour stocker et rechercher efficacement les identifiants d'usines
  - Arbre classique pour modéliser le réseau de distribution
- [ ] Planifier les algorithmes pour:
  - Calcul des histogrammes (max, src, real)
  - Calcul des pertes d'eau (leaks)
- [ ] Dessiner un diagramme de flux de données

## 2. Implémentation du script Shell

### 2.1 Structure de base
- [ ] Créer un script principal avec gestion des arguments
- [ ] Implémenter la vérification des arguments (`histo max`, `histo src`, `histo real`, `leaks <id>`)
- [ ] Ajouter la vérification de la présence et compilation du programme C
- [ ] Implémenter la mesure du temps d'exécution

### 2.2 Intégration avec le programme C
- [ ] Développer l'appel au programme C avec les bons arguments
- [ ] Gérer les codes de retour et messages d'erreur
- [ ] Traiter les sorties du programme C

### 2.3 Génération des graphiques
- [ ] Implémenter l'utilisation de GnuPlot pour générer les histogrammes
- [ ] Créer les scripts GnuPlot pour visualiser les 5 plus grandes et 5 plus petites valeurs

## 3. Développement du programme C

### 3.1 Organisation du code
- [ ] Créer une structure modulaire:
  - Fichiers .h pour les déclarations
  - Fichiers .c pour les implémentations
  - Makefile pour la compilation

### 3.2 Implémentation des structures de données
- [ ] Développer la structure AVL pour les identifiants d'usines
- [ ] Implémenter l'arbre de modélisation du réseau de distribution
- [ ] Créer les fonctions d'insertion, recherche et parcours

### 3.3 Fonctionnalité "histogramme"
- [ ] Parser le fichier CSV efficacement
- [ ] Calculer les trois types de volumes:
  - Capacité maximale par usine
  - Volume total capté (sources → usine)
  - Volume réellement traité (avec pertes)
- [ ] Générer le fichier CSV de sortie (trié par ID d'usine en ordre alphabétique inverse)

### 3.4 Fonctionnalité "pertes d'eau"
- [ ] Implémenter l'algorithme de parcours du réseau aval d'une usine
- [ ] Calculer la répartition de l'eau et les pertes à chaque tronçon
- [ ] Générer le fichier d'historique des rendements

### 3.5 Gestion des erreurs et optimisation
- [ ] Implémenter une gestion robuste des erreurs (codes de retour)
- [ ] Optimiser l'utilisation de la mémoire
- [ ] Libérer correctement toutes les allocations dynamiques

## 4. Tests et validation

### 4.1 Tests unitaires
- [ ] Tester chaque module C individuellement
- [ ] Vérifier le bon fonctionnement du script Shell avec différents arguments
- [ ] Tester les cas limites et erreurs

### 4.2 Tests d'intégration
- [ ] Tester l'application complète sur un petit échantillon de données
- [ ] Valider sur le fichier CSV complet
- [ ] Tester avec un fichier CSV différent pour éviter les solutions "codées en dur"

### 4.3 Validation des résultats
- [ ] Vérifier la cohérence des histogrammes générés
- [ ] Valider les calculs de pertes d'eau
- [ ] S'assurer que les fichiers de sortie respectent le format attendu

## 5. Fonctionnalités bonus

### 5.1 Histogramme cumulé
- [ ] Implémenter la commande `histo all`
- [ ] Créer un histogramme combinant les 3 volumes (capacité, captage, traitement)

### 5.2 Analyse avancée des fuites
- [ ] Identifier le tronçon avec la plus grande perte absolue

## 6. Documentation et livrables

### 6.1 GitHub
- [ ] Créer un dépôt GitHub
- [ ] Établir une routine de commits (avant et après chaque séance de TD)
- [ ] Organiser le dépôt selon les bonnes pratiques

### 6.2 Documentation
- [ ] Rédiger un README détaillé avec instructions d'utilisation
- [ ] Créer un document PDF décrivant:
  - La répartition des tâches
  - Le planning suivi
  - Les limitations du projet
- [ ] Documenter le code (commentaires)

### 6.3 Dossier de tests
- [ ] Préparer un dossier "tests" avec des exemples reproductibles
- [ ] Documenter les procédures de test

## 7. Planning et suivi

### 7.1 Planification
- [ ] Établir un calendrier des tâches
- [ ] Définir des jalons intermédiaires
- [ ] Répartir les responsabilités entre les membres de l'équipe

### 7.2 Suivi
- [ ] Effectuer des réunions régulières de suivi
- [ ] Mettre à jour l'avancement dans le dépôt GitHub
- [ ] Ajuster le planning si nécessaire

## Rappel des exigences clés
- Code correctement commenté et indenté
- Robustesse (pas de crash, gestion des erreurs)
- Libération de toute la mémoire allouée
- Respect des formats de sortie demandés
- Commits réguliers (avant/après chaque TD)