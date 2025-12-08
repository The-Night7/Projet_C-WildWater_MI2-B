#!/bin/bash

# --- 0. Setup ---
cd "$(dirname "$0")/.." || exit 1
DATA_DIR="data"
SRC_DIR="src"
GRAPH_DIR="graphs"
EXEC_MAIN="$SRC_DIR/c-wildwater"
INPUT_DATA="$DATA_DIR/c-wildwater_v3.dat"

mkdir -p "$GRAPH_DIR" "$DATA_DIR"

if [ ! -f "$EXEC_MAIN" ]; then
    echo "Compilation..."
    make -C "$SRC_DIR" clean && make -C "$SRC_DIR"
fi

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 [histo <max|src|real>] | [leaks <ID>]"
    exit 1
fi

COMMAND="$1"
PARAM="$2"

if [ "$COMMAND" = "histo" ]; then
    echo "--- Mode Histo ($PARAM) ---"
    OUT_CSV="$DATA_DIR/vol_${PARAM}.csv"
    
    # Appel C + Tri numérique croissant
    "$EXEC_MAIN" "$INPUT_DATA" "$PARAM" | LC_NUMERIC=C sort -t';' -k2,2n > "$OUT_CSV"
    
    if [ ! -s "$OUT_CSV" ]; then
        echo "Erreur: CSV vide."
        exit 1
    fi

    # --- 10 Plus Grands (Fin du fichier) ---
    GP_BIG="$DATA_DIR/data_big.dat"
    tail -n 10 "$OUT_CSV" > "$GP_BIG"
    IMG_BIG="$GRAPH_DIR/vol_${PARAM}_big.png"

    gnuplot -persist <<-EOF
        set terminal png size 1200,800
        set output '$IMG_BIG'
        set title "Top 10 Stations ($PARAM) - M.m3"
        set style data histograms
        set style fill solid 1.0 border -1
        set datafile separator ";"
        set ylabel "M.m3"
        set xtics rotate by -45
        plot '$GP_BIG' using 2:xtic(1) title "Volume" lc rgb "blue"
EOF
    echo "Image Top 10 : $IMG_BIG"

    # --- 50 Plus Petits (Début du fichier) ---
    GP_SMALL="$DATA_DIR/data_small.dat"
    head -n 50 "$OUT_CSV" > "$GP_SMALL"
    IMG_SMALL="$GRAPH_DIR/vol_${PARAM}_small.png"

    gnuplot -persist <<-EOF
        set terminal png size 1600,900
        set output '$IMG_SMALL'
        set title "Bottom 50 Stations ($PARAM) - M.m3"
        set style data histograms
        set style fill solid 1.0 border -1
        set datafile separator ";"
        set ylabel "M.m3"
        set xtics rotate by -90 font ",8"
        plot '$GP_SMALL' using 2:xtic(1) title "Volume" lc rgb "red"
EOF
    echo "Image Bottom 50 : $IMG_SMALL"
    
    rm "$GP_BIG" "$GP_SMALL"

elif [ "$COMMAND" = "leaks" ]; then
    echo "--- Mode Leaks ($PARAM) ---"
    LEAK_FILE="$DATA_DIR/leaks.dat"
    # Supprimer leaks.dat s'il existe pour repartir propre (ou garder >> selon besoin)
    # rm -f "$LEAK_FILE" 

    VAL=$("$EXEC_MAIN" "$INPUT_DATA" "$PARAM")
    
    if [ "$VAL" = "0" ] || [ -z "$VAL" ]; then
        echo "Usine introuvable."
        echo "$PARAM;-1" >> "$LEAK_FILE"
    else
        echo "Fuites: $VAL M.m3"
        echo "$PARAM;$VAL" >> "$LEAK_FILE"
    fi
fi