#!/bin/bash

# -----------------------------------------------------------------------------
# Script principal pour le projet C‑WildWater
#
# Ce script compile le projet si nécessaire et exécute le binaire « c‑wildwater »
# pour générer des histogrammes ou calculer les pertes en aval d’une usine.
# Les histogrammes sont exportés au format CSV et peuvent être convertis en
# graphiques PNG à l’aide de gnuplot.
# -----------------------------------------------------------------------------

# Se placer à la racine du projet (le dossier parent du script)
cd "$(dirname "$0")/.." || exit 1

DATA_DIR="data"
SRC_DIR="src"
GRAPH_DIR="data/output_images"
EXEC_MAIN="$SRC_DIR/c-wildwater"

# Exemple de nom de fichier d’entrée par défaut (modifiable)
INPUT_DATA="$DATA_DIR/c-wildwater_v3.dat"

# Créer les répertoires si nécessaire
mkdir -p "$GRAPH_DIR" "$DATA_DIR"

# Compiler si l’exécutable n’existe pas
if [ ! -f "$EXEC_MAIN" ]; then
    echo "Compilation..."
    make -C "$SRC_DIR" clean && make -C "$SRC_DIR"
fi

# Vérification des arguments
if [ "$#" -lt 2 ]; then
    echo "Usage : $0 [histo <max|src|real>] | [leaks <ID>]"
    exit 1
fi

COMMAND="$1"
PARAM="$2"

if [ "$COMMAND" = "histo" ]; then
    echo "--- Mode Histo ($PARAM) ---"
    OUT_CSV="$DATA_DIR/vol_${PARAM}.csv"
    # Exécuter le programme et trier par valeur ascendante (colonne 2)
    "$EXEC_MAIN" "$INPUT_DATA" "$PARAM" | LC_ALL=C sort -t';' -k2,2g > "$OUT_CSV"
    if [ ! -s "$OUT_CSV" ]; then
        echo "Erreur : CSV vide."
        exit 1
    fi
    # Top 10
    GP_BIG="$DATA_DIR/data_big.dat"
    tail -n 10 "$OUT_CSV" > "$GP_BIG"
    IMG_BIG="$GRAPH_DIR/vol_${PARAM}_big.png"
    gnuplot -persist <<-EOF
        set terminal png size 1200,800
        set output '$IMG_BIG'
        set title "Top 10 Stations ($PARAM) - M.m3"
        set style data histograms
        set style fill solid 1.0 border -1
        set datafile separator ';'
        set ylabel 'M.m3'
        set xtics rotate by -45
        plot '$GP_BIG' using 2:xtic(1) title 'Volume' lc rgb 'blue'
EOF
    echo "Image Top 10 : $IMG_BIG"
    # Bottom 50
    GP_SMALL="$DATA_DIR/data_small.dat"
    head -n 50 "$OUT_CSV" > "$GP_SMALL"
    IMG_SMALL="$GRAPH_DIR/vol_${PARAM}_small.png"
    gnuplot -persist <<-EOF
        set terminal png size 1600,900
        set output '$IMG_SMALL'
        set title "Bottom 50 Stations ($PARAM) - M.m3"
        set style data histograms
        set style fill solid 1.0 border -1
        set datafile separator ';'
        set ylabel 'M.m3'
        set xtics rotate by -90 font ',8'
        plot '$GP_SMALL' using 2:xtic(1) title 'Volume' lc rgb 'red'
EOF
    echo "Image Bottom 50 : $IMG_SMALL"
    # Nettoyage temporaire
    rm -f "$GP_BIG" "$GP_SMALL"

elif [ "$COMMAND" = "leaks" ]; then
    echo "--- Mode Leaks ($PARAM) ---"
    LEAK_FILE="$DATA_DIR/leaks.dat"
    VAL=$("$EXEC_MAIN" "$INPUT_DATA" "$PARAM")
    if [ "$VAL" = "0" ] || [ -z "$VAL" ]; then
        echo "Usine introuvable."
        echo "$PARAM;-1" >> "$LEAK_FILE"
    else
        echo "Fuites : $VAL M.m3"
        echo "$PARAM;$VAL" >> "$LEAK_FILE"
    fi
else
    echo "Commande inconnue.  Utilisez 'histo' ou 'leaks'."
    exit 1
fi

exit 0
