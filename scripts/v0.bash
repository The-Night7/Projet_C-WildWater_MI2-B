#!/bin/bash

# Nettoyage des fichiers dans le dossier 'logs'
if [ -d "tests/logs" ]; then
  echo "Nettoyage des fichiers dans le dossier 'logs'..."
  rm -f tests/logs/*
else
  echo "Le dossier 'tests/logs' n'existe pas. Création du dossier..."
  mkdir -p tests/logs
fi

# Vérification que le dossier 'data' existe
if [ ! -d "data" ]; then
  echo "Le dossier 'data' n'existe pas. Création du dossier..."
  mkdir -p data
fi

# Vérification que le dossier 'bin' existe
if [ ! -d "bin" ]; then
  echo "Le dossier 'bin' n'existe pas. Création du dossier..."
  mkdir -p bin
fi

# Chemin du fichier CSV
csv_file="data/c-wildwater_v0.csv"

if [ ! -f "$csv_file" ]; then
  # Création du fichier CSV et ajout de la première ligne
  echo "Usine,Amont,Aval,Volume,Taux de perte" > "$csv_file"

  echo "Le fichier $csv_file a été créé avec succès et la première ligne a été ajoutée."
fi

# Compilation et exécution du programme C
echo "Compilation du programme C..."
gcc -o bin/main src/make_csv_v0.c

if [ $? -eq 0 ]; then
  echo "Compilation réussie. Exécution du programme..."

  echo "Logs V0 !!" > tests/logs/erreurs_v0.log

  # Exécution du programme compilé
  ./bin/main 2> tests/logs/erreurs_v0.log

else
  echo "Échec de la compilation. Veuillez vérifier le code source."
  exit 1
fi

echo "Le processus est terminé."
