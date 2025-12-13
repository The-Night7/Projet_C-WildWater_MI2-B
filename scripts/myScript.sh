#!/bin/bash

# -----------------------------------------------------------------------------
# Script principal pour le projet C‑WildWater
#
# Ce script compile le projet si nécessaire et exécute le binaire «c‑wildwater»
# pour générer des histogrammes ou calculer les pertes en aval d'une usine.
# Les histogrammes sont exportés au format CSV et peuvent être convertis en
# graphiques PNG à l'aide de gnuplot.
# -----------------------------------------------------------------------------

###############################################################################
# Script d'analyse pour le projet C‑WildWater
#
# Ce script regroupe et unifie les fonctionnalités des deux scripts fournis
# initialement (`myScript.sh` et `vags.bash`).  Il prend en charge la
# compilation du projet, l'exécution en mode histogramme ou fuites, la
# génération de fichiers CSV et la création de graphiques via `gnuplot`.
#
# Utilisation:
#   ./scripts/myScript.sh [<fichier_donnees>] <mode> <argument>
#
#  * Si `<fichier_donnees>` est omis, le fichier par défaut
#    `data/c-wildwater_v3.dat` est utilisé.
#  * `<mode>` doit être `histo` ou `leaks`.
#  * En mode `histo`, `<argument>` est `max`, `src` ou `real`.
#  * En mode `leaks`, `<argument>` est l'identifiant d'une usine ou `all`.
###############################################################################

# Se placer à la racine du projet (le dossier parent du script)
cd "$(dirname "$0")/.." || exit 1

# Répertoires utilisés
DATA_DIR="data"
SRC_DIR="src"
GRAPH_DIR="$DATA_DIR/output_images"
EXEC_MAIN="$SRC_DIR/c-wildwater"
LOG_FILE="$DATA_DIR/processing.log"

# Fichier d'entrée par défaut
DEFAULT_INPUT="$DATA_DIR/c-wildwater_v3.dat"

# Créer les répertoires nécessaires
mkdir -p "$GRAPH_DIR" "$DATA_DIR"

# Fonction d'affichage de l'aide
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

# Fonction pour tracer l'avancement
log_progress() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

# Vérification du nombre minimal d'arguments
if [ "$#" -lt 2 ]; then
    usage
fi

# Initialiser le fichier de log
echo "=== Démarrage du traitement $(date '+%Y-%m-%d %H:%M:%S') ===" > "$LOG_FILE"

# Déterminer si le premier argument est un fichier ou un mode
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

# Vérification que le fichier de données existe
if [ ! -f "$DATAFILE" ]; then
    log_progress "Erreur: le fichier '$DATAFILE' n'existe pas."
    exit 1
fi

# Afficher la taille du fichier d'entrée
FILE_SIZE=$(du -h "$DATAFILE" | cut -f1)
FILE_LINES=$(wc -l < "$DATAFILE")
log_progress "Taille du fichier: $FILE_SIZE, Nombre de lignes: $FILE_LINES"

# Compilation du binaire si nécessaire
if [ ! -x "$EXEC_MAIN" ]; then
    log_progress "--- Compilation du programme ---"
    (cd "$SRC_DIR" && make clean && make) || {
        log_progress "Erreur: échec de la compilation."
        exit 1
    }
    log_progress "Compilation terminée avec succès"
fi

# S'assurer que le binaire est exécutable
chmod +x "$EXEC_MAIN" 2>/dev/null
log_progress "Vérification des permissions du binaire: $EXEC_MAIN"

# Exécution selon le mode
case "$COMMAND" in
    histo)
        # Vérification du paramètre d'histogramme
        case "$PARAM" in
            max|src|real)
                ;;
            *)
                log_progress "Erreur: le mode d'histogramme doit être 'max', 'src' ou 'real'."
                exit 1
                ;;
        esac
        log_progress "--- Mode Histogramme ($PARAM) ---"
        OUT_CSV="$DATA_DIR/vol_${PARAM}.csv"
        # Exécuter le programme et trier par valeur ascendante (colonne 2)
        log_progress "Exécution du programme: $EXEC_MAIN $DATAFILE $PARAM"
        "$EXEC_MAIN" "$DATAFILE" "$PARAM" | LC_ALL=C sort -t';' -k2,2g > "$OUT_CSV"
        if [ ! -s "$OUT_CSV" ]; then
            log_progress "Erreur: CSV vide."
            exit 1
        fi
        log_progress "Génération du CSV terminée: $(wc -l < "$OUT_CSV") lignes écrites"

        # Générer le graphique des 10 plus grandes valeurs
        GP_BIG="$DATA_DIR/data_big.dat"
        tail -n 10 "$OUT_CSV" > "$GP_BIG"
        IMG_BIG="$GRAPH_DIR/vol_${PARAM}_big.png"
        log_progress "Génération du graphique Top 10..."
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
        log_progress "Image Top 10 générée: $IMG_BIG"

        # Générer le graphique des 50 plus petites valeurs
        GP_SMALL="$DATA_DIR/data_small.dat"
        head -n 50 "$OUT_CSV" > "$GP_SMALL"
        IMG_SMALL="$GRAPH_DIR/vol_${PARAM}_small.png"
        log_progress "Génération du graphique Bottom 50..."
        gnuplot -persist <<EOF
            set terminal png size 1600,900
            set output '$IMG_SMALL'
            set title "Bottom 50 Stations ($PARAM) - M.m3"
            set key outside top center horizontal
            set offset 0, 0, graph 0.1, graph 0.05
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
        log_progress "Image Bottom 50 générée: $IMG_SMALL"

        # Nettoyage des fichiers temporaires
        rm -f "$GP_BIG" "$GP_SMALL"
        log_progress "Nettoyage des fichiers temporaires"
        ;;
    leaks)
        log_progress "--- Mode Fuites ($PARAM) ---"
        LEAK_FILE="$DATA_DIR/leaks.dat"
        CACHE_FILE="$DATA_DIR/.leaks_cache.dat"

        # Créer le fichier cache s'il n'existe pas
        touch "$CACHE_FILE"
        log_progress "Vérification du fichier cache: $CACHE_FILE"

        # Mode "all" pour calculer toutes les fuites
        if [ "$PARAM" = "all" ]; then
            log_progress "Calcul des fuites pour toutes les usines..."

            # Vérifier que le programme C supporte le mode 'all'
            if ! grep -q "define ALL_LEAKS" "$SRC_DIR/main.c" 2>/dev/null; then
                log_progress "Le programme C ne supporte pas le mode 'all'. Veuillez implémenter cette fonctionnalité."
                exit 1
            fi
            log_progress "Support du mode 'all' détecté dans le code source"

            # Exécuter le programme avec l'option "all"
            START_TIME=$SECONDS
            log_progress "Exécution de: $EXEC_MAIN $DATAFILE all"

            # Créer un fichier temporaire pour suivre la progression
            PROGRESS_FILE="$DATA_DIR/.progress.tmp"

            # Exécuter le programme en arrière-plan et suivre stderr
            "$EXEC_MAIN" "$DATAFILE" "all" > "$LEAK_FILE.new" 2> >(tee "$PROGRESS_FILE" >&2) &
            PID=$!

            # Suivre la progression en temps réel
            log_progress "Traitement en cours (PID: $PID)..."
            while kill -0 $PID 2>/dev/null; do
                if [ -f "$PROGRESS_FILE" ]; then
                    PROGRESS=$(grep -o "Lignes traitees : [0-9]*" "$PROGRESS_FILE" | tail -1)
                    if [ -n "$PROGRESS" ]; then
                        log_progress "$PROGRESS"
                    fi
                fi
                sleep 5
            done

            # Attendre la fin du processus et récupérer le code de retour
            wait $PID
            RESULT_CODE=$?
            rm -f "$PROGRESS_FILE"

            CALC_TIME=$((SECONDS - START_TIME))
            log_progress "Temps de calcul: ${CALC_TIME}s, Code de retour: $RESULT_CODE"

            if [ $RESULT_CODE -ne 0 ]; then
                log_progress "Erreur: le programme a échoué avec le code de retour $RESULT_CODE."
                exit 1
            fi

            # Vérifier que le fichier de résultat n'est pas vide
            if [ ! -s "$LEAK_FILE.new" ]; then
                log_progress "Erreur: aucun résultat généré."
                exit 1
            fi

            # Mettre à jour le fichier de fuites et le cache
            mv "$LEAK_FILE.new" "$LEAK_FILE"
            cp "$LEAK_FILE" "$CACHE_FILE"

            RESULT_LINES=$(wc -l < "$LEAK_FILE")
            log_progress "Calcul terminé en ${CALC_TIME}s. Résultats enregistrés dans $LEAK_FILE"
            log_progress "Nombre d'usines traitées: $RESULT_LINES"

        # Vérifier si l'usine a déjà été calculée (cache)
        elif [ -f "$CACHE_FILE" ]; then
            CACHED_VAL=$(grep -F "$(echo "$PARAM" | sed 's/;/\\;/g')" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
            if [ -n "$CACHED_VAL" ]; then
                log_progress "Résultat en cache pour '$PARAM': $CACHED_VAL M.m3"
                # Mise à jour du fichier de résultats si nécessaire
                if ! grep -q "$(echo "$PARAM" | sed 's/;/\\;/g')" "$LEAK_FILE" 2>/dev/null; then
                    echo "$PARAM;$CACHED_VAL" >> "$LEAK_FILE"
                    log_progress "Mise à jour du fichier de résultats avec la valeur en cache"
                fi
                exit 0
            fi
            log_progress "Pas de résultat en cache pour '$PARAM'"
        fi

        # Si plusieurs usines sont spécifiées (séparées par des virgules) et ce n'est pas "all"
        if [[ "$PARAM" == *","* ]]; then
            log_progress "Traitement par lot de plusieurs usines..."
            IFS=',' read -ra FACILITIES <<< "$PARAM"
            log_progress "Nombre d'usines à traiter: ${#FACILITIES[@]}"

            for FAC in "${FACILITIES[@]}"; do
                # Suppression des espaces avant/après
                FAC=$(echo "$FAC" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
                log_progress "Traitement de l'usine: '$FAC'"

                # Vérifier dans le cache
                CACHED_VAL=$(grep -F "$(echo "$FAC" | sed 's/;/\\;/g')" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
                if [ -n "$CACHED_VAL" ]; then
                    log_progress "[$FAC] Résultat en cache: $CACHED_VAL M.m3"
                    # Mise à jour du fichier de résultats si nécessaire
                    if ! grep -q "$(echo "$FAC" | sed 's/;/\\;/g')" "$LEAK_FILE" 2>/dev/null; then
                        echo "$FAC;$CACHED_VAL" >> "$LEAK_FILE"
                        log_progress "Mise à jour du fichier de résultats avec la valeur en cache"
                    fi
                    continue
                }

                # Calcul des fuites pour cette usine
                START_TIME=$SECONDS
                log_progress "Exécution de: $EXEC_MAIN $DATAFILE \"$FAC\""
                VAL=$("$EXEC_MAIN" "$DATAFILE" "$FAC" 2> >(tee -a "$LOG_FILE"))
                CALC_TIME=$((SECONDS - START_TIME))

                # Traitement du résultat
                if [ "$VAL" = "-1" ]; then
                    log_progress "[$FAC] Usine introuvable (${CALC_TIME}s)"
                    echo "$FAC;-1" >> "$LEAK_FILE"
                    echo "$FAC;-1" >> "$CACHE_FILE"
                else
                    log_progress "[$FAC] Fuites: $VAL M.m3 (calculé en ${CALC_TIME}s)"
                    echo "$FAC;$VAL" >> "$LEAK_FILE"
                    echo "$FAC;$VAL" >> "$CACHE_FILE"
                fi
            done
        # Traitement d'une seule usine (si ce n'est pas "all")
        elif [ "$PARAM" != "all" ]; then
            # Traitement d'une seule usine
            START_TIME=$SECONDS
            log_progress "Exécution de: $EXEC_MAIN $DATAFILE \"$PARAM\""
            VAL=$("$EXEC_MAIN" "$DATAFILE" "$PARAM" 2> >(tee -a "$LOG_FILE"))
            CALC_TIME=$((SECONDS - START_TIME))

            # Si le programme C renvoie -1, l'usine est introuvable.  Dans
            # tous les autres cas, la valeur représente le volume de pertes en
            # millions de m³.
            if [ "$VAL" = "-1" ]; then
                log_progress "Usine introuvable (recherche en ${CALC_TIME}s)."
                echo "$PARAM;-1" >> "$LEAK_FILE"
                echo "$PARAM;-1" >> "$CACHE_FILE"
            else
                log_progress "Fuites: $VAL M.m3 (calculé en ${CALC_TIME}s)"
                echo "$PARAM;$VAL" >> "$LEAK_FILE"
                echo "$PARAM;$VAL" >> "$CACHE_FILE"
            fi
        fi

        # Optimisation du fichier de fuites (élimination des doublons)
        if [ -f "$LEAK_FILE" ]; then
            log_progress "Optimisation du fichier de fuites (élimination des doublons)"
            sort -u -t';' -k1,1 "$LEAK_FILE" > "${LEAK_FILE}.tmp"
            mv "${LEAK_FILE}.tmp" "$LEAK_FILE"
            log_progress "Fichier de fuites optimisé: $(wc -l < "$LEAK_FILE") lignes"
        fi
        ;;
    *)
        log_progress "Erreur: commande inconnue ('$COMMAND').  Utilisez 'histo' ou 'leaks'."
        exit 1
        ;;
esac

TOTAL_TIME=$SECONDS
log_progress "Durée totale du traitement: ${TOTAL_TIME}s"
echo "=== Fin du traitement $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$LOG_FILE"

exit 0