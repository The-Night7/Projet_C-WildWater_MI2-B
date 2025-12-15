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
#     * En mode 'histo': max, src, real ou all
#     * En mode 'leaks': identifiant d'usine, liste d'usines séparées par virgules, ou 'all' (bonus)
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

# Couleurs pour l'affichage
BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
BOLD='\033[1m'
RESET='\033[0m'

# Affiche l'aide sur l'utilisation du script
usage() {
    cat <<EOF
    ${BOLD}Utilisation:${RESET} $0 [<fichier_donnees>] <histo|leaks> <paramètre>

      ${BOLD}<fichier_donnees>${RESET}  Chemin du fichier .dat ou .csv (optionnel).
                          Si absent, utilise $DEFAULT_INPUT.
      ${BOLD}histo${RESET}              Génère un histogramme (paramètre: max|src|real|all).
      ${BOLD}leaks${RESET}              Calcule les pertes pour l'usine donnée ou toutes les usines.
                          Paramètre: nom d'usine, liste d'usines séparées par virgules,
                          ou "all" pour toutes les usines.

    ${BOLD}Exemples:${RESET}
      $0 histo max
      $0 histo all
      $0 data/mon_fichier.dat histo src
      $0 leaks Facility\ complex\ #RH400057F
      $0 data/mon_fichier.dat leaks "Facility complex #RH400057F"
      $0 leaks "Facility A,Facility B,Facility C"
      $0 leaks all
EOF
    exit 1
}

# Affiche une barre de progression
show_progress() {
    local current=$1
    local total=$2
    local width=50
    local percent=$((current * 100 / total))
    local completed=$((width * current / total))
    
    printf "\r[%-${width}s] %3d%%" "$(printf "%0.s#" $(seq 1 $completed))" "$percent"
}

# Enregistre et affiche les messages de progression
log_progress() {
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "${BLUE}[${timestamp}]${RESET} $1" | tee -a "$LOG_FILE"
}

# Affiche un message d'erreur
log_error() {
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "${RED}[${timestamp}] ERREUR:${RESET} $1" | tee -a "$LOG_FILE"
}

# Affiche un message de succès
log_success() {
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo -e "${GREEN}[${timestamp}] SUCCÈS:${RESET} $1" | tee -a "$LOG_FILE"
}

# Vérification des arguments
if [ "$#" -lt 2 ]; then
    usage
fi

# Initialisation du log
echo "=== Démarrage du traitement $(date '+%Y-%m-%d %H:%M:%S') ===" > "$LOG_FILE"

# Début du chronomètre global (en ms)
GLOBAL_START=$(date +%s%3N)

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

log_progress "Mode: ${BOLD}$COMMAND${RESET}, Paramètre: ${BOLD}$PARAM${RESET}, Fichier: ${BOLD}$DATAFILE${RESET}"

# Vérification de l'existence du fichier de données
if [ ! -f "$DATAFILE" ]; then
    log_error "Le fichier '$DATAFILE' n'existe pas."
    exit 1
fi

# Compilation du programme si nécessaire
if [ ! -x "$EXEC_MAIN" ]; then
    log_progress "Compilation du programme en cours..."
    echo -ne "${YELLOW}"
    (cd "$SRC_DIR" && make clean && make) || {
        echo -ne "${RESET}"
        log_error "Échec de la compilation."
        exit 1
    }
    echo -ne "${RESET}"
    log_success "Compilation terminée avec succès"
fi

chmod +x "$EXEC_MAIN" 2>/dev/null

# Fonction pour afficher une séparation visuelle
show_separator() {
    echo -e "\n${YELLOW}----------------------------------------${RESET}\n"
}

# Traitement selon le mode choisi
case "$COMMAND" in
    histo)
        # Vérification du paramètre d'histogramme
        case "$PARAM" in
            max|src|real|all) ;;
            *)
                log_error "Le mode d'histogramme doit être 'max', 'src', 'real' ou 'all'."
                exit 1
                ;;
        esac

        show_separator
        log_progress "Mode Histogramme (${BOLD}$PARAM${RESET})"

        # Génération du fichier CSV
        OUT_CSV="$DATA_DIR/vol_${PARAM}.csv"
        echo -e "${YELLOW}Génération des données en cours...${RESET}"
        
        # Exécution avec barre de progression simulée
        "$EXEC_MAIN" "$DATAFILE" "$PARAM" > "$DATA_DIR/.temp_output" &
        PID=$!
        
        # Affichage d'une barre de progression pendant le traitement
        i=0
        spin='-\|/'
        while kill -0 $PID 2>/dev/null; do
            i=$(( (i+1) % 4 ))
            printf "\r[%c] Traitement des données..." "${spin:$i:1}"
            sleep 0.1
        done
        
        wait $PID
        RESULT_CODE=$?
        
        if [ $RESULT_CODE -ne 0 ]; then
            printf "\r%s\n" "$(printf ' %.0s' {1..50})"  # Efface la ligne
            log_error "Échec de la génération des données."
            exit 1
        fi
        
        # Tri des données
        printf "\r[#] Tri des données...                 "
        LC_ALL=C sort -t';' -k2,2g "$DATA_DIR/.temp_output" > "$OUT_CSV"
        rm -f "$DATA_DIR/.temp_output"
        printf "\r%s\n" "$(printf ' %.0s' {1..50})"  # Efface la ligne

        if [ ! -s "$OUT_CSV" ]; then
            log_error "CSV vide (Aucune donnée générée)."
            exit 1
        fi
        
        log_success "Données générées et triées avec succès"
        
        # --- BRANCHEMENT : MODE BONUS "ALL" ou MODES CLASSIQUES ---
        if [ "$PARAM" = "all" ]; then
            # ---------------------------------------------------------
            # BONUS 1 : Histogramme combiné (Capacité / Source / Réel)
            # ---------------------------------------------------------
            IMG_ALL="$GRAPH_DIR/vol_all.png"
            GP_DATA="$DATA_DIR/data_all.dat"
            
            echo -e "${YELLOW}Création de l'histogramme combiné...${RESET}"
            
            # On garde les 50 plus grandes stations (fin du fichier trié)
            tail -n 50 "$OUT_CSV" > "$GP_DATA"
            
            gnuplot -persist <<EOF
set terminal png size 1600,900
set output '$IMG_ALL'
set title "Histogramme Cumulé (Capacité vs Pertes vs Réel)"
set style data histograms
set style histogram rowstacked
set style fill solid 1.0 border -1
set datafile separator ';'
set ylabel 'Volume (M.m3)'
set xtics rotate by -45 font ',8'
set key outside top center horizontal
set grid y
# Rappel colonnes CSV : 1:Nom, 2:Capacité, 3:Source, 4:Réel
# Empilement (Bas -> Haut) :
# 1. Bleu : Réel (col 4)
# 2. Rouge : Perte (Source - Réel) -> (\$3-\$4)
# 3. Vert : Capacité Restante (Max - Source) -> (\$2-\$3)
plot '$GP_DATA' using 4:xtic(1) title 'Réel (Sortie)' lc rgb '#3366CC', \
'' using (\$3-\$4) title 'Pertes (Transport)' lc rgb '#DC3912', \
'' using (\$2-\$3) title 'Capacité Non Utilisée' lc rgb '#109618'
EOF
            log_success "Graphique combiné généré : ${BOLD}$IMG_ALL${RESET}"
            rm -f "$GP_DATA"
        else
            # ---------------------------------------------------------
            # MODES CLASSIQUES (max, src, real)
            # ---------------------------------------------------------
            echo -e "${YELLOW}Création des histogrammes...${RESET}"
            
            # 1. Top 10 (Les plus grands)
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
            log_success "Image Top 10 générée: ${BOLD}$IMG_BIG${RESET}"

            # 2. Bottom 50 (Les plus petits)
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
            log_success "Image Bottom 50 générée: ${BOLD}$IMG_SMALL${RESET}"
            
            # Nettoyage des fichiers temporaires
            rm -f "$GP_BIG" "$GP_SMALL"
        fi
        ;;

    leaks)
        show_separator
        log_progress "Mode Fuites (${BOLD}$PARAM${RESET})"
        LEAK_FILE="$DATA_DIR/leaks.dat"
        CACHE_FILE="$DATA_DIR/.leaks_cache.dat"
        touch "$CACHE_FILE"

        # Fonction interne pour traiter une usine
        process_factory() {
            local FACTORY="$1"
            # Nettoyage espaces
            FACTORY=$(echo "$FACTORY" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
            
            echo -e "\n${YELLOW}Traitement de l'usine:${RESET} ${BOLD}$FACTORY${RESET}"
            
            # Check Cache
            CACHED_VAL=$(grep -F "$(echo "$FACTORY" | sed 's/;/\\;/g')" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
            if [ -n "$CACHED_VAL" ]; then
                echo -e "${GREEN}[CACHE]${RESET} Résultat trouvé en cache"
                if ! grep -q "$(echo "$FACTORY" | sed 's/;/\\;/g')" "$LEAK_FILE" 2>/dev/null; then
                    echo "$FACTORY;$CACHED_VAL" >> "$LEAK_FILE"
                fi
                echo -e "${BOLD}Volume de fuites pour $FACTORY:${RESET} ${BLUE}${CACHED_VAL} M.m3${RESET}"
                return
            fi
            
            # Calcul C avec barre de progression
            echo -e "${YELLOW}Calcul en cours...${RESET}"
            T_START=$(date +%s%3N)
            
            # Utilisation d'un fichier temporaire pour capturer la sortie d'erreur
            TEMP_ERR_FILE="$DATA_DIR/.temp_err_$$"
            
            # Exécution du programme en arrière-plan
            "$EXEC_MAIN" "$DATAFILE" "$FACTORY" 2> "$TEMP_ERR_FILE" &
            PID=$!
            
            # Affichage d'une barre de progression animée
            i=0
            spin='-\|/'
            while kill -0 $PID 2>/dev/null; do
                i=$(( (i+1) % 4 ))
                
                # Récupérer les informations de progression si disponibles
                if [ -f "$TEMP_ERR_FILE" ]; then
                    PROGRESS_INFO=$(grep -o "Lignes traitees : [0-9]*" "$TEMP_ERR_FILE" | tail -n1)
                    if [ -n "$PROGRESS_INFO" ]; then
                        printf "\r[%c] %s" "${spin:$i:1}" "$PROGRESS_INFO"
                    else
                        printf "\r[%c] Traitement en cours..." "${spin:$i:1}"
                    fi
                else
                    printf "\r[%c] Traitement en cours..." "${spin:$i:1}"
                fi
                
                sleep 0.1
            done
            
            # Attendre la fin du processus et récupérer la valeur
            wait $PID
            VAL=$(cat "$DATA_DIR/.temp_output" 2>/dev/null)
            if [ -z "$VAL" ]; then
                VAL=$("$EXEC_MAIN" "$DATAFILE" "$FACTORY" 2>/dev/null)
            fi
            
            T_END=$(date +%s%3N)
            DUREE=$((T_END - T_START))
            
            # Effacer la ligne de progression
            printf "\r%s\n" "$(printf ' %.0s' {1..70})"
            
            if [ "$VAL" = "-1" ]; then
                log_error "Usine '$FACTORY' introuvable (${DUREE}ms)"
                echo "$FACTORY;-1" >> "$LEAK_FILE"
                echo "$FACTORY;-1" >> "$CACHE_FILE"
            else
                log_success "Calcul terminé en ${DUREE}ms"
                echo "$FACTORY;$VAL" >> "$LEAK_FILE"
                echo "$FACTORY;$VAL" >> "$CACHE_FILE"
                echo -e "${BOLD}Volume de fuites pour $FACTORY:${RESET} ${BLUE}${VAL} M.m3${RESET}"
            fi
            
            # Nettoyage
            rm -f "$TEMP_ERR_FILE" "$DATA_DIR/.temp_output"
        }

        # Mode "all": calcul pour toutes les usines
        if [ "$PARAM" = "all" ]; then
            # Vérification du support dans le code source
            if ! grep -q "define ALL_LEAKS" "$SRC_DIR/main.c" 2>/dev/null; then
                log_error "Le programme C ne supporte pas le mode 'all'."
                exit 1
            fi

            # Exécution et suivi de la progression
            echo -e "${YELLOW}Calcul des fuites pour toutes les usines...${RESET}"
            START_TIME=$SECONDS
            PROGRESS_FILE="$DATA_DIR/.progress.tmp"

            "$EXEC_MAIN" "$DATAFILE" "all" > "$LEAK_FILE.new" 2> "$PROGRESS_FILE" &
            PID=$!

            # Affichage d'une barre de progression animée
            i=0
            spin='-\|/'
            while kill -0 $PID 2>/dev/null; do
                i=$(( (i+1) % 4 ))
                
                # Récupérer les informations de progression si disponibles
                if [ -f "$PROGRESS_FILE" ]; then
                    PROGRESS_INFO=$(grep -o "Lignes traitees : [0-9]*" "$PROGRESS_FILE" | tail -n1)
                    if [ -n "$PROGRESS_INFO" ]; then
                        printf "\r[%c] %s" "${spin:$i:1}" "$PROGRESS_INFO"
                    else
                        printf "\r[%c] Traitement en cours..." "${spin:$i:1}"
                    fi
                else
                    printf "\r[%c] Traitement en cours..." "${spin:$i:1}"
                fi
                
                sleep 0.1
            done

            # Effacer la ligne de progression
            printf "\r%s\n" "$(printf ' %.0s' {1..70})"
            
            wait $PID
            RESULT_CODE=$?
            rm -f "$PROGRESS_FILE"

            if [ $RESULT_CODE -ne 0 ] || [ ! -s "$LEAK_FILE.new" ]; then
                log_error "Échec du calcul des fuites."
                exit 1
            fi

            # Mise à jour des fichiers de résultats
            mv "$LEAK_FILE.new" "$LEAK_FILE"
            cp "$LEAK_FILE" "$CACHE_FILE"

            # Calcul et affichage du volume total des fuites
            TOTAL_LEAKS=$(awk -F';' '{if (NF==2) s+=$2} END {printf "%.6f", s}' "$LEAK_FILE")
            echo -e "\n${BOLD}Volume total de fuites:${RESET} ${BLUE}${TOTAL_LEAKS} M.m3${RESET}"
            log_success "Calcul pour toutes les usines terminé"

        # Traitement d'usines multiples (séparées par virgules)
        elif [[ "$PARAM" == *","* ]]; then
            IFS=',' read -ra FACTORIES <<< "$PARAM"
            TOTAL_FACTORIES=${#FACTORIES[@]}
            
            echo -e "${YELLOW}Traitement de $TOTAL_FACTORIES usines...${RESET}"
            
            for ((i=0; i<TOTAL_FACTORIES; i++)); do
                FAC="${FACTORIES[$i]}"
                echo -e "\n${YELLOW}[$((i+1))/$TOTAL_FACTORIES]${RESET}"
                process_factory "$FAC"
            done
            
            # Affichage d'un résumé
            echo -e "\n${BOLD}Résumé des volumes de fuites:${RESET}"
            TOTAL_LEAKS=0
            for FAC in "${FACTORIES[@]}"; do
                FAC=$(echo "$FAC" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
                VAL=$(grep -F "$(echo "$FAC" | sed 's/;/\\;/g')" "$LEAK_FILE" | tail -n1 | cut -d';' -f2)
                if [ -n "$VAL" ] && [ "$VAL" != "-1" ]; then
                    echo -e "- ${BOLD}$FAC:${RESET} ${BLUE}${VAL} M.m3${RESET}"
                    TOTAL_LEAKS=$(echo "$TOTAL_LEAKS + $VAL" | bc)
                fi
            done
            echo -e "\n${BOLD}Total des fuites:${RESET} ${BLUE}${TOTAL_LEAKS} M.m3${RESET}"
            
        # Traitement d'une seule usine
        else
            process_factory "$PARAM"
        fi

        # Optimisation du fichier de fuites (élimination des doublons)
        if [ -f "$LEAK_FILE" ]; then
            sort -u -t';' -k1,1 "$LEAK_FILE" > "${LEAK_FILE}.tmp"
            mv "${LEAK_FILE}.tmp" "$LEAK_FILE"
        fi
        ;;

    *)
        log_error "Commande inconnue ('$COMMAND'). Utilisez 'histo' ou 'leaks'."
        exit 1
        ;;
esac

# Fin du chronomètre global
GLOBAL_END=$(date +%s%3N)
TOTAL_DUREE=$((GLOBAL_END - GLOBAL_START))

show_separator
log_success "Traitement terminé en ${BOLD}${TOTAL_DUREE} ms${RESET}"
echo "=== Fin du traitement $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$LOG_FILE"

exit 0