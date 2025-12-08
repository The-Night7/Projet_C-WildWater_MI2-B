#!/bin/bash

# ===================================================================
# Script de gestion pour le projet C-WildWater
# Rôle : Vérifier l'environnement, compiler et lancer l'analyse
#        et générer des graphiques avec gnuplot
# ===================================================================

# --- Configuration des répertoires ---
cd "$(dirname "$0")/.." || exit 1
PROJECT_DIR="src"
DATA_DIR="data"
GRAPH_DIR="graphs"
EXECUTABLE="$PROJECT_DIR/c-wildwater"
mkdir -p "$GRAPH_DIR" "$DATA_DIR"

# --- 1. Vérification des arguments ---
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

# --- 2. Déterminer le mode d'analyse ---
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

# --- 3. Vérification de l'existence du fichier de données ---
if [ ! -f "$INPUT_FILE" ]; then
    echo "Erreur : Le fichier '$INPUT_FILE' n'existe pas."
    exit 1
fi

# --- 4. Compilation du projet ---
echo "--- Compilation en cours ---"
if [ ! -f "$EXECUTABLE" ]; then
    (cd "$PROJECT_DIR" && make clean && make)
else
    # Recompilation si demandée
    (cd "$PROJECT_DIR" && make)
fi

# Vérifier si la compilation a réussi
if [ $? -ne 0 ]; then
    echo "Erreur : La compilation a échoué."
    exit 1
fi
echo "--- Compilation réussie ---"

# --- 5. Exécution et traitement ---
echo "--- Lancement de l'analyse ---"

if [ "$OPERATION_TYPE" = "histogram" ]; then
    echo "--- Mode Histogramme ($ANALYSIS_MODE) ---"
    OUT_CSV="$DATA_DIR/vol_${ANALYSIS_MODE}.csv"
    
    # Exécution du programme C avec les arguments corrects
    # prog <input.csv> <output.csv> <command> <option>
    "$EXECUTABLE" "$INPUT_FILE" "$OUT_CSV" "histo" "$ANALYSIS_MODE"
    
    # Vérifier si le fichier de sortie a été créé
    if [ ! -f "$OUT_CSV" ] || [ ! -s "$OUT_CSV" ]; then
        echo "Erreur: Fichier CSV de sortie vide ou non généré."
        exit 1
    fi
    
    # --- Génération des graphiques pour les 10 plus grands volumes ---
    GP_BIG="$DATA_DIR/data_big.dat"
    LC_NUMERIC=C sort -t';' -k2,2n "$OUT_CSV" | tail -n 10 > "$GP_BIG"
    IMG_BIG="$GRAPH_DIR/vol_${ANALYSIS_MODE}_big.png"

    gnuplot -persist <<-EOF
        set terminal png size 1200,800
        set output '$IMG_BIG'
        set title "Top 10 Stations ($ANALYSIS_MODE) - M.m3"
        set style data histograms
        set style fill solid 1.0 border -1
        set datafile separator ";"
        set ylabel "M.m3"
        set xtics rotate by -45
        plot '$GP_BIG' using 2:xtic(1) title "Volume" lc rgb "blue"
EOF
    echo "Image Top 10 : $IMG_BIG"

    # --- Génération des graphiques pour les 50 plus petits volumes ---
    GP_SMALL="$DATA_DIR/data_small.dat"
    LC_NUMERIC=C sort -t';' -k2,2n "$OUT_CSV" | head -n 50 > "$GP_SMALL"
    IMG_SMALL="$GRAPH_DIR/vol_${ANALYSIS_MODE}_small.png"

    gnuplot -persist <<-EOF
        set terminal png size 1600,900
        set output '$IMG_SMALL'
        set title "Bottom 50 Stations ($ANALYSIS_MODE) - M.m3"
        set style data histograms
        set style fill solid 1.0 border -1
        set datafile separator ";"
        set ylabel "M.m3"
        set xtics rotate by -90 font ",8"
        plot '$GP_SMALL' using 2:xtic(1) title "Volume" lc rgb "red"
EOF
    echo "Image Bottom 50 : $IMG_SMALL"

    # Nettoyage des fichiers temporaires
    rm -f "$GP_BIG" "$GP_SMALL"
    
    # Afficher un aperçu du rapport
    echo "--- Rapport généré: $OUT_CSV ---"
    echo "Aperçu du rapport (5 premières lignes):"
    head -n 5 "$OUT_CSV"

elif [ "$OPERATION_TYPE" = "leaks" ]; then
    echo "--- Mode Leaks ($FACTORY_ID) ---"
    LEAK_FILE="$DATA_DIR/leaks_history.csv"
    
    # Exécution du calcul des fuites avec les arguments corrects
    # prog <input.csv> <output.csv> <command> <option>
    "$EXECUTABLE" "$INPUT_FILE" "$LEAK_FILE" "leaks" "$FACTORY_ID"
    
    # Vérification du fichier d'historique des rendements
    if [ -f "$LEAK_FILE" ]; then
        echo "--- Historique des rendements mis à jour: $LEAK_FILE ---"
        echo "Dernière entrée:"
        tail -n 1 "$LEAK_FILE"
    else
        echo "Attention: Aucun fichier d'historique n'a été généré."
    fi
fi

# --- 6. Fin ---
echo "--- Terminé ---"