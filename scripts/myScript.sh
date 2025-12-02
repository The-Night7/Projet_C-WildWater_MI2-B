#!/bin/bash

# --- 0. Initialisation ---
# Se placer à la racine du projet (un niveau au-dessus du dossier scripts)
cd "$(dirname "$0")/.." || exit 1

DATA_DIR="data"
SRC_DIR="src"
GRAPH_DIR="graphs"
EXEC_MAIN="$SRC_DIR/c-wildwater"
EXEC_CONV="$SRC_DIR/converter"

# --- 1. Vérifications Arguments ---
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 histo <max|src|real>"
    exit 1
fi

COMMAND="$1"
PARAM="$2"

if [ "$COMMAND" != "histo" ]; then
    echo "Erreur: Commande '$COMMAND' non supportée."
    exit 1
fi

if [[ "$PARAM" != "max" && "$PARAM" != "src" && "$PARAM" != "real" ]]; then
    echo "Erreur: Paramètre '$PARAM' invalide. Choix: max, src, real."
    exit 1
fi

mkdir -p "$GRAPH_DIR" "$DATA_DIR"

# --- 2. Compilation ---
echo "--- Compilation ---"
if [ ! -f "$SRC_DIR/Makefile" ]; then
    echo "Erreur: Makefile introuvable dans $SRC_DIR."
    exit 1
fi

# On force la recompilation propre
make -C "$SRC_DIR" > /dev/null
if [ $? -ne 0 ]; then
    echo "Erreur de compilation."
    exit 1
fi

# --- 3. Pré-traitement (Fusion v0 + v3) ---
echo "--- Nettoyage et Fusion des données ---"

INPUT_V0="$DATA_DIR/c-wildwater_v0.dat"
INPUT_V3="$DATA_DIR/c-wildwater_v3.dat"
CLEAN_CSV="$DATA_DIR/tmp_clean.csv"
FULL_OUTPUT_CSV="$DATA_DIR/vol_${PARAM}.csv"

# On vide le fichier propre
> "$CLEAN_CSV"

# Traitement V0
if [ -f "$INPUT_V0" ]; then
    echo "Traitement de $INPUT_V0..."
    "$EXEC_CONV" "$INPUT_V0" "tmp_v0.csv"
    cat "tmp_v0.csv" >> "$CLEAN_CSV"
    rm "tmp_v0.csv"
else
    echo "Attention: $INPUT_V0 introuvable."
fi

# Traitement V3
if [ -f "$INPUT_V3" ]; then
    echo "Traitement de $INPUT_V3 (Cela peut prendre du temps)..."
    "$EXEC_CONV" "$INPUT_V3" "tmp_v3.csv"
    cat "tmp_v3.csv" >> "$CLEAN_CSV"
    rm "tmp_v3.csv"
else
    echo "Attention: $INPUT_V3 introuvable."
fi

if [ ! -s "$CLEAN_CSV" ]; then
    echo "Erreur: Aucun fichier de données valide trouvé ou fichiers vides."
    exit 1
fi

# --- 4. Calculs C (AVL) ---
echo "--- Analyse C (Mode: $PARAM) ---"
start_time=$(date +%s)

"$EXEC_MAIN" "$CLEAN_CSV" "$PARAM" > "$FULL_OUTPUT_CSV"

if [ $? -ne 0 ]; then
    echo "Erreur lors de l'exécution du programme C."
    exit 1
fi

end_time=$(date +%s)
duration=$((end_time - start_time))
echo "Traitement terminé en ${duration}s. Sortie : $FULL_OUTPUT_CSV"

# --- 5. Graphique (Top 5 / Bottom 5) ---
GRAPH_DATA="$DATA_DIR/graph_temp.dat"
SORTED_TEMP="$DATA_DIR/sorted_temp.csv"
OUTPUT_IMG="$GRAPH_DIR/vol_${PARAM}.png"

# Tri numérique sur la quantité (colonne 2)
sort -t';' -k2 -n "$FULL_OUTPUT_CSV" > "$SORTED_TEMP"

# Extraction 5 min + 5 max
nb_lines=$(wc -l < "$SORTED_TEMP")
if [ "$nb_lines" -le 10 ]; then
    cat "$SORTED_TEMP" > "$GRAPH_DATA"
else
    head -n 5 "$SORTED_TEMP" > "$GRAPH_DATA"
    tail -n 5 "$SORTED_TEMP" >> "$GRAPH_DATA"
fi

echo "--- Génération Graphique ---"
gnuplot -persist <<-EOFMarker
    set terminal png size 1200,800
    set output '$OUTPUT_IMG'
    set title "Histogramme des stations - Mode $PARAM"
    set style data histograms
    set style fill solid 1.0 border -1
    set boxwidth 0.7
    set ylabel "Volume (m3)"
    set xlabel "Usines"
    set grid y
    set xtics rotate by -45 font ",8"
    set datafile separator ";"
    plot '$GRAPH_DATA' using 2:xtic(1) title "$PARAM" linecolor rgb "#3498db"
EOFMarker

echo "Image générée : $OUTPUT_IMG"

# Nettoyage temporaire
rm -f "$CLEAN_CSV" "$SORTED_TEMP" "$GRAPH_DATA"