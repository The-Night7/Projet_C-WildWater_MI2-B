#!/bin/bash

# ===================================================================
# Script de gestion pour le projet C-WildWater
# Rôle : Vérifier l'environnement, compiler et lancer l'analyse.
# ===================================================================

# 1. Vérification des arguments
if [ "$#" -lt 2 ]; then
    echo "Erreur : Arguments manquants."
    echo "Usage: $0 <fichier_donnees.dat ou .csv> <mode>"
    echo "Modes disponibles:"
    echo "  histo max  : Histogramme basé sur la capacité maximale"
    echo "  histo src  : Histogramme basé sur le volume capté"
    echo "  histo real : Histogramme basé sur le volume réellement traité"
    echo "  histo all  : Histogramme combiné (bonus)"
    echo "  leaks <identifiant_usine> : Calcul des pertes pour une usine donnée"
    exit 1
fi

INPUT_FILE="$1"
COMMAND="$2"
PROJECT_DIR="../src"
EXECUTABLE="../src/c-wildwater"

# Déterminer le mode d'analyse
if [ "$COMMAND" = "histo" ]; then
    if [ "$#" -lt 3 ]; then
        echo "Erreur : Mode d'histogramme manquant."
        echo "Usage: $0 <fichier_donnees.dat ou .csv> histo <mode>"
        echo "Modes disponibles: max, src, real, all"
        exit 1
    fi

    ANALYSIS_MODE="$3"

    # Vérification du mode d'histogramme
    if [[ ! "$ANALYSIS_MODE" =~ ^(max|src|real|all)$ ]]; then
        echo "Erreur : Mode d'histogramme invalide. Utilisez max, src, real ou all."
    exit 1
fi

    OPERATION_TYPE="histogram"
elif [ "$COMMAND" = "leaks" ]; then
    if [ "$#" -lt 3 ]; then
        echo "Erreur : Identifiant d'usine manquant."
        echo "Usage: $0 <fichier_donnees.dat ou .csv> leaks <identifiant_usine>"
    exit 1
fi

    FACTORY_ID="$3"
    ANALYSIS_MODE="leaks"
    OPERATION_TYPE="leaks"
else
    echo "Erreur : Commande non reconnue. Utilisez 'histo' ou 'leaks'."
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
echo "--- Lancement de l'analyse ---"
if [ "$OPERATION_TYPE" = "histogram" ]; then
    echo "Mode: histogramme ($ANALYSIS_MODE)"
    $EXECUTABLE "$INPUT_FILE" "histo" "$ANALYSIS_MODE"

    # 5. Vérification du fichier de sortie
    OUTPUT_FILE="../scripts/output_histo_${ANALYSIS_MODE}.csv"
    if [ -f "$OUTPUT_FILE" ]; then
        echo "--- Rapport généré: $OUTPUT_FILE ---"
        # Afficher les 5 premières lignes du rapport
        echo "Aperçu du rapport:"
        head -n 5 "$OUTPUT_FILE"
    else
        echo "Attention: Aucun fichier de sortie n'a été généré."
    fi
else
    echo "Mode: calcul des fuites pour l'usine $FACTORY_ID"
    $EXECUTABLE "$INPUT_FILE" "leaks" "$FACTORY_ID"

    # Vérification du fichier d'historique des rendements
    OUTPUT_FILE="../scripts/leaks_history.csv"
    if [ -f "$OUTPUT_FILE" ]; then
        echo "--- Historique des rendements mis à jour: $OUTPUT_FILE ---"
        # Afficher la dernière ligne du rapport
        echo "Dernière entrée:"
        tail -n 1 "$OUTPUT_FILE"
    fi
fi

# 6. Fin
echo "--- Terminé ---"