#!/bin/bash

# ===================================================================
# Script de gestion pour le projet C-WildWater
# Rôle : Vérifier l'environnement, compiler et lancer l'analyse.
# ===================================================================

# 1. Vérification des arguments
if [ "$#" -lt 2 ]; then
    echo "Erreur : Arguments manquants."
    echo "Usage: $0 <fichier_donnees.dat ou .csv> <mode>"
    echo "Modes disponibles: max, src, real"
    exit 1
fi

INPUT_FILE="$1"
ANALYSIS_MODE="$2"
EXECUTABLE="../src/c-wildwater"
PROJECT_DIR="../src"

# Vérification du mode
if [[ ! "$ANALYSIS_MODE" =~ ^(max|src|real)$ ]]; then
    echo "Erreur : Mode d'analyse invalide. Utilisez max, src ou real."
    exit 1
fi

# 2. Vérification de l'existence du fichier de données
if [ ! -f "$INPUT_FILE" ]; then
    echo "Erreur : Le fichier '$INPUT_FILE' n'existe pas."
    exit 1
fi

# 3. Compilation du projet (Nettoyage + Compilation)
echo "--- Compilation en cours ---"
(cd $PROJECT_DIR && make re)

# Vérifier si la compilation a réussi (code de retour 0)
if [ $? -ne 0 ]; then
    echo "Erreur : La compilation a échoué."
    exit 1
fi

echo "--- Compilation réussie ---"

# 4. Exécution du programme C
echo "--- Lancement de l'analyse (mode: $ANALYSIS_MODE) ---"
$EXECUTABLE "$INPUT_FILE" "$ANALYSIS_MODE"

# 5. Vérification du fichier de sortie
OUTPUT_FILE="../scripts/output_${ANALYSIS_MODE}.csv"
if [ -f "$OUTPUT_FILE" ]; then
    echo "--- Rapport généré: $OUTPUT_FILE ---"
    # Afficher les 5 premières lignes du rapport
    echo "Aperçu du rapport:"
    head -n 5 "$OUTPUT_FILE"
else
    echo "Attention: Aucun fichier de sortie n'a été généré."
fi
# 6. Fin
echo "--- Terminé ---"