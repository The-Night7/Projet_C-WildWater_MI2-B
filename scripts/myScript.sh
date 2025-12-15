#!/bin/bash

# -----------------------------------------------------------------------------
# Script d'analyse pour le projet C‑WildWater
#
# Fonctionnalités:
# - Compilation du projet si nécessaire
# - Génération d'histogrammes (mode 'histo')
# - Calcul des pertes en aval d'usines (mode 'leaks')
# - Export CSV et création de graphiques PNG via gnuplot
#
# Utilisation: ./scripts/myScript.sh [<fichier_donnees>] <mode> <argument>
#   - <fichier_donnees>: Optionnel, par défaut data/c-wildwater_v3.dat
#   - <mode>: 'histo' (histogramme) ou 'leaks' (fuites)
#   - <argument>:
#     * En mode 'histo': max, src ou real
#     * En mode 'leaks': identifiant d'usine ou 'all'
# -----------------------------------------------------------------------------

# Positionnement à la racine du projet
cd "$(dirname "$0")/.." || exit 1

# Configuration des chemins et fichiers
DATA_DIR="data"
SRC_DIR="src"
BIN_DIR="src/bin"
GRAPH_DIR="$DATA_DIR/output_images"
EXEC_MAIN="$BIN_DIR/c-wildwater"
LOG_FILE="$DATA_DIR/processing.log"
DEFAULT_INPUT="$DATA_DIR/c-wildwater_v3.dat"

# Création des répertoires nécessaires
mkdir -p "$GRAPH_DIR" "$DATA_DIR" "$BIN_DIR"

# Affiche l'aide sur l'utilisation du script
usage() {
    cat <<EOF
    Utilisation: $0 [<fichier_donnees>] <histo|leaks> <paramètre>

      <fichier_donnees>  Chemin du fichier .dat ou .csv (optionnel).
                          Si absent, utilise $DEFAULT_INPUT.
      histo              Génère un histogramme (paramètre: max|src|real).
      leaks              Calcule les pertes pour l'usine donnée ou toutes les usines.
                          Paramètre: nom d'usine ou "all" pour toutes les usines.

    Exemples:
      $0 histo max
      $0 data/mon_fichier.dat histo src
      $0 leaks Facility\ complex\ #RH400057F
      $0 data/mon_fichier.dat leaks "Facility complex #RH400057F"
      $0 leaks all
EOF
    exit 1
}

# Enregistre et affiche les messages de progression
log_progress() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

# Vérification des arguments
if [ "$#" -lt 2 ]; then
    usage
fi

# Initialisation du log
echo "=== Démarrage du traitement $(date '+%Y-%m-%d %H:%M:%S') ===" > "$LOG_FILE"

# Analyse des arguments: fichier de données et mode
if [ "$1" = "histo" ] || [ "$1" = "leaks" ]; then
    DATAFILE="$DEFAULT_INPUT"
    COMMAND="$1"
    PARAM="$2"
    shift 2
else
    DATAFILE="$1"
    COMMAND="$2"
    PARAM="$3"
    shift 3
fi

log_progress "Mode: $COMMAND, Paramètre: $PARAM, Fichier: $DATAFILE"

# Vérification de l'existence du fichier de données
if [ ! -f "$DATAFILE" ]; then
    log_progress "Erreur: le fichier '$DATAFILE' n'existe pas."
    exit 1
fi

# Compilation du programme si nécessaire
if [ ! -x "$EXEC_MAIN" ]; then
    log_progress "Compilation du programme..."
    (cd "$SRC_DIR" && make clean && make) || {
        log_progress "Erreur: échec de la compilation."
        exit 1
    }
    log_progress "Compilation terminée"
fi

chmod +x "$EXEC_MAIN" 2>/dev/null

# Traitement selon le mode choisi
case "$COMMAND" in
    histo)
        # Vérification du paramètre d'histogramme
        case "$PARAM" in
            max|src|real) ;;
            *)
                log_progress "Erreur: le mode d'histogramme doit être 'max', 'src' ou 'real'."
                exit 1
                ;;
        esac

        # Génération du fichier CSV
        OUT_CSV="$DATA_DIR/vol_${PARAM}.csv"
        "$EXEC_MAIN" "$DATAFILE" "$PARAM" | LC_ALL=C sort -t';' -k2,2g > "$OUT_CSV"

        if [ ! -s "$OUT_CSV" ]; then
            log_progress "Erreur: CSV vide."
            exit 1
        fi

        # Création des graphiques avec gnuplot
        # 1. Top 10 des plus grandes valeurs
        GP_BIG="$DATA_DIR/data_big.dat"
        tail -n 10 "$OUT_CSV" > "$GP_BIG"
        IMG_BIG="$GRAPH_DIR/vol_${PARAM}_big.png"

        gnuplot -persist <<EOF
            set terminal png size 1200,800
            set output '$IMG_BIG'
            set title "Top 10 Stations ($PARAM) - M.m3"
            set yrange [0:*]
            set style data histograms
            set style fill solid 1.0 border -1
            set datafile separator ';'
            set ylabel 'M.m3'
            set xtics rotate by -45
            plot '$GP_BIG' using 2:xtic(1) title 'Volume' lc rgb 'blue'
EOF

        # 2. Bottom 50 des plus petites valeurs
        GP_SMALL="$DATA_DIR/data_small.dat"
        head -n 50 "$OUT_CSV" > "$GP_SMALL"
        IMG_SMALL="$GRAPH_DIR/vol_${PARAM}_small.png"

        gnuplot -persist <<EOF
            set terminal png size 1600,900
            set output '$IMG_SMALL'
            set title "Bottom 50 Stations ($PARAM) - M.m3"
            set key outside top center horizontal
            set style data histograms
            set boxwidth 0.8 relative
            set style fill solid 1.0 border -1
            set datafile separator ';'
            set ylabel 'Volume (M.m3)'
            set format y "%.4f"
            set yrange [*:*]
            set xtics rotate by -90 font ',8'
            plot '$GP_SMALL' using 2:xtic(1) title 'Volume' lc rgb 'red'
EOF

        # Nettoyage des fichiers temporaires
        rm -f "$GP_BIG" "$GP_SMALL"
        ;;

    leaks)
        LEAK_FILE="$DATA_DIR/leaks.dat"
        CACHE_FILE="$DATA_DIR/.leaks_cache.dat"
        touch "$CACHE_FILE"

        # Mode "all": calcul pour toutes les usines
        if [ "$PARAM" = "all" ]; then
            # Vérification du support dans le code source
            if ! grep -q "define ALL_LEAKS" "$SRC_DIR/main.c" 2>/dev/null; then
                log_progress "Le programme C ne supporte pas le mode 'all'."
                exit 1
            fi

            # Exécution et suivi de la progression
            START_TIME=$SECONDS
            PROGRESS_FILE="$DATA_DIR/.progress.tmp"

            "$EXEC_MAIN" "$DATAFILE" "all" > "$LEAK_FILE.new" 2> >(tee "$PROGRESS_FILE" >&2) &
            PID=$!

            # Attente de la fin du processus
            while kill -0 $PID 2>/dev/null; do
                sleep 1
            done

            wait $PID
            RESULT_CODE=$?
            rm -f "$PROGRESS_FILE"

            if [ $RESULT_CODE -ne 0 ] || [ ! -s "$LEAK_FILE.new" ]; then
                log_progress "Erreur lors du calcul des fuites."
                exit 1
            fi

            # Mise à jour des fichiers de résultats
            mv "$LEAK_FILE.new" "$LEAK_FILE"
            cp "$LEAK_FILE" "$CACHE_FILE"

            # Calcul et affichage du volume total des fuites
        awk -F';' '{if (NF==2) s+=$2} END {printf "Volume total de fuites: %.6fM.m3\n", s}' "$LEAK_FILE"

        # Traitement d'usines multiples (séparées par virgules)
        elif [[ "$PARAM" == *","* ]]; then
            IFS=',' read -ra FACILITIES <<< "$PARAM"

            for FAC in "${FACILITIES[@]}"; do
                # Suppression des espaces
                FAC=$(echo "$FAC" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')

                # Vérification du cache
                CACHED_VAL=$(grep -F "$(echo "$FAC" | sed 's/;/\\;/g')" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
                if [ -n "$CACHED_VAL" ]; then
                    # Utilisation de la valeur en cache
                    if ! grep -q "$(echo "$FAC" | sed 's/;/\\;/g')" "$LEAK_FILE" 2>/dev/null; then
                        echo "$FAC;$CACHED_VAL" >> "$LEAK_FILE"
                    fi
                    continue
                fi

                # Calcul des fuites pour cette usine
                VAL=$("$EXEC_MAIN" "$DATAFILE" "$FAC" 2> >(tee -a "$LOG_FILE"))

                # Enregistrement du résultat
                echo "$FAC;$VAL" >> "$LEAK_FILE"
                echo "$FAC;$VAL" >> "$CACHE_FILE"
            done

            # Affichage des volumes de fuites pour chaque usine spécifiée
            for FAC in "${FACILITIES[@]}"; do
                FAC=$(echo "$FAC" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
                VAL=$(grep -F "$(echo "$FAC" | sed 's/;/\\;/g')" "$LEAK_FILE" | tail -n1 | cut -d';' -f2)
                if [ -n "$VAL" ]; then
                    echo "Volume de fuites pour $FAC: ${VAL}M.m3"
                fi
            done

        # Traitement d'une seule usine
        elif [ "$PARAM" != "all" ]; then
            # Vérification du cache
            CACHED_VAL=$(grep -F "$(echo "$PARAM" | sed 's/;/\\;/g')" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
            if [ -n "$CACHED_VAL" ]; then
                if ! grep -q "$(echo "$PARAM" | sed 's/;/\\;/g')" "$LEAK_FILE" 2>/dev/null; then
                    echo "$PARAM;$CACHED_VAL" >> "$LEAK_FILE"
                fi
                VAL=$CACHED_VAL
            else
                # Calcul des fuites
                VAL=$("$EXEC_MAIN" "$DATAFILE" "$PARAM" 2> >(tee -a "$LOG_FILE"))
                echo "$PARAM;$VAL" >> "$LEAK_FILE"
                echo "$PARAM;$VAL" >> "$CACHE_FILE"
            fi

            # Affichage du volume de fuites pour l'usine spécifiée
            echo "Volume de fuites pour $PARAM: ${VAL}M.m3"
        fi

        # Optimisation du fichier de fuites (élimination des doublons)
        if [ -f "$LEAK_FILE" ]; then
            sort -u -t';' -k1,1 "$LEAK_FILE" > "${LEAK_FILE}.tmp"
            mv "${LEAK_FILE}.tmp" "$LEAK_FILE"
        fi
        ;;

    *)
        log_progress "Erreur: commande inconnue ('$COMMAND'). Utilisez 'histo' ou 'leaks'."
        exit 1
        ;;
esac

log_progress "Traitement terminé"
echo "=== Fin du traitement $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$LOG_FILE"

exit 0