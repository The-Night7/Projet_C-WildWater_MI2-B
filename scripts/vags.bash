#!/bin/bash

# ===================================================================
# Script de gestion pour le projet C-WildWater
# Rôle : Vérifier l'environnement, compiler et lancer l'analyse.
# ===================================================================

# 1. Vérification des arguments
if [ "$#" -ne 1 ]; then
    echo "Erreur : Argument manquant."
    echo "Usage: $0 <fichier_donnees.dat>"
    exit 1
fi

INPUT_FILE="$1"
EXECUTABLE="./c-wildwater"

# 2. Vérification de l'existence du fichier de données
if [ ! -f "$INPUT_FILE" ]; then
    echo "Erreur : Le fichier '$INPUT_FILE' n'existe pas."
    exit 1
fi

# 3. Compilation du projet (Nettoyage + Compilation)
echo "--- Compilation en cours ---"
make re

# Vérifier si la compilation a réussi (code de retour 0)
if [ $? -ne 0 ]; then
    echo "Erreur : La compilation a échoué."
    exit 1
fi

echo "--- Compilation réussie ---"

# 4. Exécution du programme C
echo "--- Lancement de l'analyse ---"
$EXECUTABLE "$INPUT_FILE"

# 5. Fin
echo "--- Terminé ---"
