#!/bin/bash
<<<<<<< HEAD
<<<<<<< HEAD
# Le "Shebang" ci-dessus indique au système d'utiliser l'interpréteur Bash.

# ==============================================================================
# 0. INITIALISATION ET SÉCURITÉ
# ==============================================================================

# Se placer dans le répertoire parent du script pour que les chemins relatifs fonctionnent
# $(dirname "$0") récupère le dossier où se trouve le script.
# "|| exit 1" arrête le script si le changement de dossier échoue.
cd "$(dirname "$0")/.." || exit 1

# Définition des constantes (Chemins) pour faciliter la maintenance
DATA_DIR="data"
SRC_DIR="src"
GRAPH_DIR="graphs"
EXEC_MAIN="$SRC_DIR/c-wildwater"
INPUT_DATA="$DATA_DIR/c-wildwater_v3.dat" # Fichier source (taille réelle)

# Création des dossiers de sortie s'ils n'existent pas (-p évite les erreurs si déjà là)
mkdir -p "$GRAPH_DIR" "$DATA_DIR"

# ==============================================================================
# 1. VÉRIFICATION ET COMPILATION
# ==============================================================================

# Si l'exécutable C n'existe pas, on lance la compilation.
if [ ! -f "$EXEC_MAIN" ]; then
    echo "Exécutable introuvable. Compilation en cours..."
    # On nettoie (clean) d'abord pour être sûr d'avoir une compilation propre
    make -C "$SRC_DIR" clean && make -C "$SRC_DIR"
    
    # Vérification post-compilation : si ça a échoué, on arrête tout.
    if [ ! -f "$EXEC_MAIN" ]; then
        echo "Erreur fatale : La compilation a échoué."
        exit 1
    fi
fi

# ==============================================================================
# 2. GESTION DES ARGUMENTS
# ==============================================================================

# Vérifie qu'il y a au moins 2 arguments (ex: "histo" et "max")
# $# représente le nombre d'arguments passés au script.
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 [histo <max|src|real>] | [leaks <ID>]"
    exit 1
fi

COMMAND="$1" # Premier argument : le mode (histo ou leaks)
PARAM="$2"   # Deuxième argument : le type (max/src/real) ou l'ID de l'usine

# ==============================================================================
# 3. TRAITEMENT : MODE HISTOGRAMME
# ==============================================================================

if [ "$COMMAND" = "histo" ]; then
    echo "--- Mode Histo ($PARAM) : Démarrage ---"
    
    OUT_CSV="$DATA_DIR/vol_${PARAM}.csv"
    
    # --- PIPELINE IMPORTANT ---
    # 1. "$EXEC_MAIN" ... : Exécute le programme C qui crache les données sur la sortie standard (stdout)
    # 2. | (Pipe)         : Passe le résultat directement à la commande suivante sans stocker sur disque
    # 3. sort             : Trie les lignes
    #    -t';'            : Définit le séparateur de colonnes (point-virgule)
    #    -k2,2n           : Trie sur la 2ème colonne (le volume), en mode numérique (n)
    # 4. > "$OUT_CSV"     : Écrit le résultat final trié dans le fichier CSV
    # LC_ALL=C force l'utilisation du point comme séparateur décimal (sécurité pour le tri)
    "$EXEC_MAIN" "$INPUT_DATA" "$PARAM" | LC_ALL=C sort -t';' -k2,2n > "$OUT_CSV"
            
    # Vérifie si le fichier généré est vide (signe d'un problème dans le C)
    if [ ! -s "$OUT_CSV" ]; then
        echo "Erreur : Le fichier CSV généré est vide ou n'existe pas."
        exit 1
    fi

    # --- PRÉPARATION DES DONNÉES POUR GNUPLOT ---

    # 10 Plus Grands : On prend les 10 dernières lignes du fichier trié (tail)
    GP_BIG="$DATA_DIR/data_big.dat"
    tail -n 10 "$OUT_CSV" > "$GP_BIG"
    IMG_BIG="$GRAPH_DIR/vol_${PARAM}_big.png"

    # Appel de Gnuplot avec un "Heredoc" (<<-EOF ... EOF)
    # Cela permet d'écrire les commandes Gnuplot directement dans le script shell.
    gnuplot -persist <<-EOF
        set terminal png size 1200,800            # Format de sortie image
        set output '$IMG_BIG'                     # Nom du fichier image
        set title "Top 10 Stations ($PARAM)"      # Titre du graphique
        set yrange [0:*]                          # Axe Y automatique à partir de 0
        set style data histograms                 # Type de graphique (barres)
        set style fill solid 1.0 border -1        # Remplissage couleur pleine
        set datafile separator ";"                # Séparateur du CSV
        set ylabel "Volume (k.m3)"                # Titre axe Y
        set xtics rotate by -45                   # Rotation du texte axe X pour lisibilité
        # plot ... using 2:xtic(1) : Utilise col 2 pour la hauteur, col 1 pour le nom
        plot '$GP_BIG' using 2:xtic(1) title "Volume" lc rgb "blue"
EOF
    echo "Graphique généré : $IMG_BIG"

    # 50 Plus Petits : On prend les 50 premières lignes du fichier trié (head)
    GP_SMALL="$DATA_DIR/data_small.dat"
    head -n 50 "$OUT_CSV" > "$GP_SMALL"
    IMG_SMALL="$GRAPH_DIR/vol_${PARAM}_small.png"

    gnuplot -persist <<-EOF
        set terminal png size 1600,900
        set output '$IMG_SMALL'
        set title "Bottom 50 Stations ($PARAM)"
        set key outside top center horizontal
        set offset 0, 0, graph 0.1, graph 0.05 
        set style data histograms
        set style fill solid 1.0 border -1
        set datafile separator ";"
        set ylabel "Volume (k.m3)"
        set xtics rotate by -90 font ",8"         # Rotation 90° et police plus petite car 50 barres
        plot '$GP_SMALL' using 2:xtic(1) title "Volume" lc rgb "red"
EOF
    echo "Graphique généré : $IMG_SMALL"
    
    # Nettoyage des fichiers temporaires (optionnel, mais propre)
    rm "$GP_BIG" "$GP_SMALL"

# ==============================================================================
# 4. TRAITEMENT : MODE LEAKS (FUITES)
# ==============================================================================

elif [ "$COMMAND" = "leaks" ]; then
    echo "--- Mode Leaks (Station : $PARAM) ---"
    LEAK_FILE="$DATA_DIR/leaks.dat"

    # Exécution du programme C et capture du résultat dans une variable
    # $() permet de récupérer ce que le programme C affiche avec printf
    VAL=$("$EXEC_MAIN" "$INPUT_DATA" "$PARAM")
    
    # Gestion du cas "Introuvable" (Le C renvoie -1 selon ta modif précédente)
    if [ "$VAL" = "-1" ]; then
        echo "Attention : Usine introuvable ou ID incorrect."
        # Ajout au fichier avec la valeur -1 comme demandé dans le sujet
        # >> permet d'ajouter à la fin du fichier sans l'écraser
        echo "$PARAM;-1" >> "$LEAK_FILE"
    else
        # Cas normal : Affichage et sauvegarde
        echo "Volume de fuites détecté : $VAL M.m3"
        echo "$PARAM;$VAL" >> "$LEAK_FILE"
    fi

# ==============================================================================
# 5. GESTION DES ERREURS (Commande inconnue)
# ==============================================================================
else
    echo "Erreur : Commande '$COMMAND' inconnue."
    echo "Commandes valides : histo, leaks"
    exit 1
fi

# Fin du script. Calcul de la durée totale (demandé par le sujet).
# Note : SECONDS est une variable automatique de Bash qui compte le temps écoulé.
echo "Durée du traitement : ${SECONDS}s"
=======

# -----------------------------------------------------------------------------
# Script principal pour le projet C‑WildWater
#
# Ce script compile le projet si nécessaire et exécute le binaire «c‑wildwater»
# pour générer des histogrammes ou calculer les pertes en aval d'une usine.
# -----------------------------------------------------------------------------

# Se placer à la racine du projet
cd "$(dirname "$0")/.." || exit 1

# Répertoires et Fichiers
DATA_DIR="data"
SRC_DIR="src"
GRAPH_DIR="$DATA_DIR/output_images"
EXEC_MAIN="$SRC_DIR/c-wildwater"
DEFAULT_INPUT="$DATA_DIR/c-wildwater_v3.dat"

# Créer les répertoires nécessaires
mkdir -p "$GRAPH_DIR" "$DATA_DIR"

# Fonction d'affichage de l'aide
usage() {
    cat <<EOF
    Utilisation: $0 [<fichier_donnees>] <histo|leaks> <paramètre>

      <fichier_donnees>  Chemin du fichier .dat (optionnel).
      histo              Génère un histogramme.
                         Paramètres: max | src | real | all
      leaks              Calcule les pertes pour l'usine donnée.
                         Paramètre: "Nom de l'usine"

    Exemples:
      $0 histo max
      $0 histo all
      $0 leaks "Facility complex #RH400057F"
=======

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
CACHE_DIR="$DATA_DIR/.cache"

# Création des répertoires nécessaires
mkdir -p "$GRAPH_DIR" "$DATA_DIR" "$BIN_DIR" "$CACHE_DIR"

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
>>>>>>> origin/main
EOF
    exit 1
}

<<<<<<< HEAD
# Vérification du nombre minimal d'arguments
=======
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
>>>>>>> origin/main
if [ "$#" -lt 2 ]; then
    usage
fi

<<<<<<< HEAD
# Gestion des arguments (Fichier optionnel)
=======
# Initialisation du log
echo "=== Démarrage du traitement $(date '+%Y-%m-%d %H:%M:%S') ===" > "$LOG_FILE"

# Début du chronomètre global (en ms)
GLOBAL_START=$(date +%s%3N)

# Analyse des arguments: fichier de données et mode
>>>>>>> origin/main
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

<<<<<<< HEAD
# Vérification fichier
if [ ! -f "$DATAFILE" ]; then
    echo "Erreur: le fichier '$DATAFILE' n'existe pas."
    exit 1
fi

# Compilation si nécessaire
if [ ! -x "$EXEC_MAIN" ]; then
    echo "--- Compilation du programme ---"
    (cd "$SRC_DIR" && make clean && make) || {
        echo "Erreur: échec de la compilation."
        exit 1
    }
fi

# Début du chronomètre global (en ms)
GLOBAL_START=$(date +%s%3N)

case "$COMMAND" in
    histo)
        # Vérification paramètre
        case "$PARAM" in
            max|src|real|all) ;;
            *) echo "Erreur: paramètre invalide pour histo (max, src, real, all)."; exit 1 ;;
        esac

        echo "--- Mode Histogramme ($PARAM) ---"
        OUT_CSV="$DATA_DIR/vol_${PARAM}.csv"

        # Exécution C + Tri
        # Note: On trie sur la colonne 2 (numérique) pour avoir un ordre croissant
        "$EXEC_MAIN" "$DATAFILE" "$PARAM" | LC_ALL=C sort -t';' -k2,2g > "$OUT_CSV"

        if [ ! -s "$OUT_CSV" ]; then
            echo "Erreur: CSV vide (Aucune donnée générée)."
            exit 1
        fi

=======
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
        
>>>>>>> origin/main
        # --- BRANCHEMENT : MODE BONUS "ALL" ou MODES CLASSIQUES ---
        if [ "$PARAM" = "all" ]; then
            # ---------------------------------------------------------
            # BONUS 1 : Histogramme combiné (Capacité / Source / Réel)
            # ---------------------------------------------------------
            IMG_ALL="$GRAPH_DIR/vol_all.png"
            GP_DATA="$DATA_DIR/data_all.dat"
<<<<<<< HEAD

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
            echo "Graphique généré : $IMG_ALL"
            rm -f "$GP_DATA"

=======
            
            echo -e "${YELLOW}Création de l'histogramme combiné...${RESET}"
            
            # On garde les 50 plus grandes stations (fin du fichier trié)
            # Vérifier le format du fichier CSV pour s'assurer qu'il contient bien 4 colonnes
            if [ $(head -n1 "$OUT_CSV" | grep -o ";" | wc -l) -eq 3 ]; then
                # Format correct: Nom;Capacité;Source;Réel
                tail -n 50 "$OUT_CSV" > "$GP_DATA"
            else
                # Le format n'est pas celui attendu, il faut vérifier la sortie du programme C
                log_error "Format du CSV incorrect. Vérification de la structure..."
                # Afficher les premières lignes pour diagnostic
                head -n 3 "$OUT_CSV" >> "$LOG_FILE"

                # Tentative de correction - Supposons que le programme C génère des données incomplètes
                # Créons un fichier temporaire avec la structure attendue
                echo "# Création d'un fichier de données de test pour le mode 'all'" >> "$LOG_FILE"

                # Exécuter séparément les commandes pour obtenir max, src et real
                # puis combiner les résultats
                TMP_MAX="$DATA_DIR/.cache/.temp_max.csv"
                TMP_SRC="$DATA_DIR/.cache/.temp_src.csv"
                TMP_REAL="$DATA_DIR/.cache/.temp_real.csv"

                echo -e "${YELLOW}Récupération des données max, src et real séparément...${RESET}"

                "$EXEC_MAIN" "$DATAFILE" "max" > "$TMP_MAX"
                "$EXEC_MAIN" "$DATAFILE" "src" > "$TMP_SRC"
                "$EXEC_MAIN" "$DATAFILE" "real" > "$TMP_REAL"

                # Combiner les données - Utiliser awk pour joindre les fichiers
                awk -F';' '
                    BEGIN { OFS=";" }
                    FILENAME == ARGV[1] { max[$1] = $2; next }
                    FILENAME == ARGV[2] { src[$1] = $2; next }
                    FILENAME == ARGV[3] {
                        if ($1 in max && $1 in src) {
                            print $1, max[$1], src[$1], $2
                        }
                    }
                ' "$TMP_MAX" "$TMP_SRC" "$TMP_REAL" | sort -t';' -k2,2gr | head -n 50 > "$GP_DATA"

                # Nettoyage des fichiers temporaires
                rm -f "$TMP_MAX" "$TMP_SRC" "$TMP_REAL"

                # Vérifier que le fichier généré contient des données
                if [ ! -s "$GP_DATA" ]; then
                    log_error "Impossible de générer des données valides pour l'histogramme combiné."
                    exit 1
                fi

                log_success "Données combinées générées avec succès"
            fi

            # Vérifier que le fichier de données est bien formaté pour gnuplot
            echo -e "${YELLOW}Vérification du format des données...${RESET}"
            if [ $(head -n1 "$GP_DATA" | awk -F';' '{print NF}') -ne 4 ]; then
                log_error "Le fichier de données n'a pas le bon format (4 colonnes attendues)."
                # Afficher un exemple des données pour diagnostic
                head -n 3 "$GP_DATA" >> "$LOG_FILE"
                exit 1
            fi

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

            # Vérifier que le graphique a bien été généré
            if [ -f "$IMG_ALL" ] && [ -s "$IMG_ALL" ]; then
                log_success "Graphique combiné généré : ${BOLD}$IMG_ALL${RESET}"
            else
                log_error "Échec de génération du graphique combiné."
                # Essayer avec une approche plus simple
                echo -e "${YELLOW}Tentative avec une approche alternative...${RESET}"

                # Créer un script gnuplot temporaire avec plus de debug
                GNUPLOT_SCRIPT="$DATA_DIR/.gnuplot_script.txt"
                cat > "$GNUPLOT_SCRIPT" <<EOF
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
# Debug - Afficher les données
print "Lecture du fichier: $GP_DATA"
stats '$GP_DATA' using 2 nooutput
print "Nombre de lignes valides pour colonne 2: ", STATS_records
stats '$GP_DATA' using 3 nooutput
print "Nombre de lignes valides pour colonne 3: ", STATS_records
stats '$GP_DATA' using 4 nooutput
print "Nombre de lignes valides pour colonne 4: ", STATS_records
# Simplifier le plot pour éviter les erreurs
plot '$GP_DATA' using 2:xtic(1) title 'Capacité' lc rgb '#109618', \
     '$GP_DATA' using 3 title 'Source' lc rgb '#DC3912', \
     '$GP_DATA' using 4 title 'Réel' lc rgb '#3366CC'
EOF

                # Exécuter gnuplot avec le script de debug
                gnuplot "$GNUPLOT_SCRIPT" 2>> "$LOG_FILE"

                if [ -f "$IMG_ALL" ] && [ -s "$IMG_ALL" ]; then
                    log_success "Graphique alternatif généré : ${BOLD}$IMG_ALL${RESET}"
                else
                    log_error "Échec de la génération du graphique alternatif."
                fi

                rm -f "$GNUPLOT_SCRIPT"
            fi

            rm -f "$GP_DATA"
>>>>>>> origin/main
        else
            # ---------------------------------------------------------
            # MODES CLASSIQUES (max, src, real)
            # ---------------------------------------------------------
<<<<<<< HEAD
=======
            echo -e "${YELLOW}Création des histogrammes...${RESET}"
>>>>>>> origin/main
            
            # 1. Top 10 (Les plus grands)
            GP_BIG="$DATA_DIR/data_big.dat"
            tail -n 10 "$OUT_CSV" > "$GP_BIG"
            IMG_BIG="$GRAPH_DIR/vol_${PARAM}_big.png"
<<<<<<< HEAD
            
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
            echo "Image Top 10: $IMG_BIG"
=======

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
>>>>>>> origin/main

            # 2. Bottom 50 (Les plus petits)
            GP_SMALL="$DATA_DIR/data_small.dat"
            head -n 50 "$OUT_CSV" > "$GP_SMALL"
            IMG_SMALL="$GRAPH_DIR/vol_${PARAM}_small.png"
<<<<<<< HEAD
            
            gnuplot -persist <<EOF
                set terminal png size 1600,900
                set output '$IMG_SMALL'
                set title "Bottom 50 Stations ($PARAM) - M.m3"
                set key outside top center horizontal
                set style data histograms
                set boxwidth 0.8 relative
                set style fill solid 1.0 border -1
                set datafile separator ';'
                set ylabel 'Volume'
                set yrange [*:*]
                set xtics rotate by -90 font ',8'
                plot '$GP_SMALL' using 2:xtic(1) title 'Volume' lc rgb 'red'
EOF
            echo "Image Bottom 50: $IMG_SMALL"
=======

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
>>>>>>> origin/main
            rm -f "$GP_BIG" "$GP_SMALL"
        fi
        ;;

    leaks)
<<<<<<< HEAD
        # ---------------------------------------------------------
        # MODE LEAKS (Calcul de fuites)
        # ---------------------------------------------------------
        echo "--- Mode Fuites ($PARAM) ---"
=======
        show_separator
        log_progress "Mode Fuites (${BOLD}$PARAM${RESET})"
>>>>>>> origin/main
        LEAK_FILE="$DATA_DIR/leaks.dat"
        CACHE_FILE="$DATA_DIR/.leaks_cache.dat"
        touch "$CACHE_FILE"

<<<<<<< HEAD
        # Fonction interne pour traiter une usine
=======
        # Fonction interne pour traiter une usine - Version optimisée
>>>>>>> origin/main
        process_factory() {
            local FACTORY="$1"
            # Nettoyage espaces
            FACTORY=$(echo "$FACTORY" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
<<<<<<< HEAD

            # Check Cache
            CACHED_VAL=$(grep -F "$(echo "$FACTORY" | sed 's/;/\\;/g')" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
            if [ -n "$CACHED_VAL" ]; then
                echo "[$FACTORY] Résultat en cache: $CACHED_VAL M.m3"
                if ! grep -q "$(echo "$FACTORY" | sed 's/;/\\;/g')" "$LEAK_FILE" 2>/dev/null; then
                    echo "$FACTORY;$CACHED_VAL" >> "$LEAK_FILE"
                fi
                return
            fi

            # Calcul C
            T_START=$(date +%s%3N)
            VAL=$("$EXEC_MAIN" "$DATAFILE" "$FACTORY")
            T_END=$(date +%s%3N)
            DUREE=$((T_END - T_START))

            if [ "$VAL" = "-1" ]; then
                echo "[$FACTORY] Usine introuvable (${DUREE}ms)"
                echo "$FACTORY;-1" >> "$LEAK_FILE"
                echo "$FACTORY;-1" >> "$CACHE_FILE"
            else
                echo "[$FACTORY] Fuites: $VAL M.m3 (calculé en ${DUREE}ms)"
                echo "$FACTORY;$VAL" >> "$LEAK_FILE"
                echo "$FACTORY;$VAL" >> "$CACHE_FILE"
            fi
        }

        if [[ "$PARAM" == *","* ]]; then
            # Traitement multiple
            IFS=',' read -ra FACTORIES <<< "$PARAM"
            for FAC in "${FACTORIES[@]}"; do
                process_factory "$FAC"
            done
        else
            # Traitement unique
            process_factory "$PARAM"
        fi

        # Nettoyage doublons
        if [ -f "$LEAK_FILE" ]; then
            sort -u -t';' -k1,1 "$LEAK_FILE" > "${LEAK_FILE}.tmp"
            mv "${LEAK_FILE}.tmp" "$LEAK_FILE"
=======
            
            echo -e "\n${YELLOW}Traitement de l'usine:${RESET} ${BOLD}$FACTORY${RESET}"
            
            # Génération d'un nom de fichier unique basé sur un hash du nom de l'usine
            # pour éviter les conflits avec les caractères spéciaux
            local FACTORY_HASH=$(echo "$FACTORY" | md5sum | cut -d' ' -f1)
            local TEMP_ERR_FILE="$CACHE_DIR/err_${FACTORY_HASH}.tmp"
            local TEMP_OUT_FILE="$CACHE_DIR/out_${FACTORY_HASH}.tmp"
            
            # Check Cache - Optimisé pour rechercher exactement le nom de l'usine
            CACHED_VAL=$(grep -F "$(echo "$FACTORY" | sed 's/;/\\;/g');" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
            if [ -n "$CACHED_VAL" ]; then
                echo -e "${GREEN}[CACHE]${RESET} Résultat trouvé en cache"
                if ! grep -q "$(echo "$FACTORY" | sed 's/;/\\;/g');" "$LEAK_FILE" 2>/dev/null; then
                    # Ajouter uniquement le résultat au format usine;valeur dans leaks.dat
                    echo "$FACTORY;$CACHED_VAL" >> "$LEAK_FILE"
                fi
                echo -e "${BOLD}Volume de fuites pour $FACTORY:${RESET} ${BLUE}${CACHED_VAL} M.m3${RESET}"
                return
            fi
            
            # Calcul C avec barre de progression - Version optimisée
            echo -e "${YELLOW}Calcul en cours...${RESET}"
            T_START=$(date +%s%3N)
            
            # Utiliser nice pour réduire la priorité du processus et ulimit pour limiter l'utilisation mémoire
            # Utiliser également stdbuf pour désactiver la mise en mémoire tampon des sorties
            nice -n 10 stdbuf -oL -eL "$EXEC_MAIN" "$DATAFILE" "$FACTORY" > "$TEMP_OUT_FILE" 2> "$TEMP_ERR_FILE" &
            PID=$!
            
            # Affichage d'une barre de progression animée avec timeout
            i=0
            spin='-\|/'
            TIMEOUT=180  # 3 minutes max
            START_TIME=$SECONDS
            
            while kill -0 $PID 2>/dev/null; do
                # Vérifier si le timeout est atteint
                ELAPSED=$((SECONDS - START_TIME))
                if [ $ELAPSED -gt $TIMEOUT ]; then
                    echo -e "\n${RED}Timeout atteint (${TIMEOUT}s). Arrêt forcé du calcul.${RESET}"
                    kill -9 $PID 2>/dev/null
                    break
                fi
                
                i=$(( (i+1) % 4 ))
                
                # Récupérer les informations de progression si disponibles
                if [ -f "$TEMP_ERR_FILE" ]; then
                    PROGRESS_INFO=$(grep -o "Lignes traitees : [0-9]*" "$TEMP_ERR_FILE" | tail -n1)
                    if [ -n "$PROGRESS_INFO" ]; then
                        printf "\r[%c] %s (${ELAPSED}s)" "${spin:$i:1}" "$PROGRESS_INFO"
                    else
                        printf "\r[%c] Traitement en cours... (${ELAPSED}s)" "${spin:$i:1}"
                    fi
                else
                    printf "\r[%c] Traitement en cours... (${ELAPSED}s)" "${spin:$i:1}"
                fi
                
                sleep 0.2
            done
            
            # Attendre la fin du processus et récupérer la valeur
            wait $PID
            EXIT_CODE=$?
            
            # Récupérer la valeur du fichier de sortie
            if [ -f "$TEMP_OUT_FILE" ]; then
                VAL=$(cat "$TEMP_OUT_FILE" | tr -d '\n\r')
            else
                VAL=""
            fi
            
            # Si le fichier est vide ou si le processus a été interrompu
            if [ -z "$VAL" ] || [ $EXIT_CODE -ne 0 ]; then
                # Tentative avec une approche plus simple et directe
                echo -e "\n${YELLOW}Nouvelle tentative avec méthode alternative...${RESET}"
                VAL=$("$EXEC_MAIN" "$DATAFILE" "$FACTORY" 2>/dev/null | tr -d '\n\r')
            fi
            
            T_END=$(date +%s%3N)
            DUREE=$((T_END - T_START))
            
            # Effacer la ligne de progression
            printf "\r%s\n" "$(printf ' %.0s' {1..70})"
            
            if [ "$VAL" = "-1" ] || [ -z "$VAL" ]; then
                log_error "Usine '$FACTORY' introuvable ou calcul échoué (${DUREE}ms)"
                # Ajouter uniquement le résultat au format usine;valeur dans leaks.dat et cache
                echo "$FACTORY;-1" >> "$LEAK_FILE"
                echo "$FACTORY;-1" >> "$CACHE_FILE"
            else
                log_success "Calcul terminé en ${DUREE}ms"
                # Ajouter uniquement le résultat au format usine;valeur dans leaks.dat et cache
                echo "$FACTORY;$VAL" >> "$LEAK_FILE"
                echo "$FACTORY;$VAL" >> "$CACHE_FILE"
                echo -e "${BOLD}Volume de fuites pour $FACTORY:${RESET} ${BLUE}${VAL} M.m3${RESET}"
            fi
            
            # Nettoyage
            rm -f "$TEMP_ERR_FILE" "$TEMP_OUT_FILE"
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
            PROGRESS_FILE="$CACHE_DIR/progress_all.tmp"
            TEMP_LEAK_FILE="$CACHE_DIR/leaks_all.tmp"

            # Exécuter le programme avec nice pour réduire sa priorité
            nice -n 10 "$EXEC_MAIN" "$DATAFILE" "all" > "$TEMP_LEAK_FILE" 2> "$PROGRESS_FILE" &
            PID=$!

            # Affichage d'une barre de progression animée avec timeout
            i=0
            spin='-\|/'
            TIMEOUT=600  # 10 minutes max
            START_PROC_TIME=$SECONDS
            
            while kill -0 $PID 2>/dev/null; do
                # Vérifier si le timeout est atteint
                ELAPSED=$((SECONDS - START_PROC_TIME))
                if [ $ELAPSED -gt $TIMEOUT ]; then
                    echo -e "\n${RED}Timeout atteint (${TIMEOUT}s). Arrêt forcé du calcul.${RESET}"
                    kill -9 $PID 2>/dev/null
                    break
                fi
                
                i=$(( (i+1) % 4 ))
                
                # Récupérer les informations de progression si disponibles
                if [ -f "$PROGRESS_FILE" ]; then
                    PROGRESS_INFO=$(grep -o "Lignes traitees : [0-9]*" "$PROGRESS_FILE" | tail -n1)
                    if [ -n "$PROGRESS_INFO" ]; then
                        printf "\r[%c] %s (${ELAPSED}s)" "${spin:$i:1}" "$PROGRESS_INFO"
                    else
                        printf "\r[%c] Traitement en cours... (${ELAPSED}s)" "${spin:$i:1}"
                    fi
                else
                    printf "\r[%c] Traitement en cours... (${ELAPSED}s)" "${spin:$i:1}"
                fi
                
                sleep 0.2
            done

            # Effacer la ligne de progression
            printf "\r%s\n" "$(printf ' %.0s' {1..70})"
            
            wait $PID
            RESULT_CODE=$?
            
            # Nettoyage
            rm -f "$PROGRESS_FILE"

            if [ $RESULT_CODE -ne 0 ] || [ ! -s "$TEMP_LEAK_FILE" ]; then
                log_error "Échec du calcul des fuites."
                rm -f "$TEMP_LEAK_FILE"
                exit 1
            fi

            # Vérifier que le fichier contient bien des données au format usine;valeur
            if grep -q ";" "$TEMP_LEAK_FILE"; then
                # Mise à jour des fichiers de résultats - Utiliser cat pour être plus rapide
                cat "$TEMP_LEAK_FILE" > "$LEAK_FILE"
                cat "$TEMP_LEAK_FILE" > "$CACHE_FILE"
            else
                log_error "Format de sortie incorrect dans le fichier temporaire."
                rm -f "$TEMP_LEAK_FILE"
                exit 1
            fi

            rm -f "$TEMP_LEAK_FILE"

            # Calcul et affichage du volume total des fuites - Utiliser awk pour plus d'efficacité
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
                VAL=$(grep -F "$(echo "$FAC" | sed 's/;/\\;/g');" "$LEAK_FILE" | tail -n1 | cut -d';' -f2)
                if [ -n "$VAL" ] && [ "$VAL" != "-1" ]; then
                    echo -e "- ${BOLD}$FAC:${RESET} ${BLUE}${VAL} M.m3${RESET}"
                    # Utiliser bc pour les calculs avec décimales
                    if command -v bc &>/dev/null; then
                        TOTAL_LEAKS=$(echo "$TOTAL_LEAKS + $VAL" | bc)
                    else
                        # Fallback si bc n'est pas disponible
                        TOTAL_LEAKS=$(awk "BEGIN {print $TOTAL_LEAKS + $VAL}")
                    fi
                fi
            done
            echo -e "\n${BOLD}Total des fuites:${RESET} ${BLUE}${TOTAL_LEAKS} M.m3${RESET}"
            
        # Traitement d'une seule usine
        else
            process_factory "$PARAM"
        fi

        # Optimisation du fichier de fuites (élimination des doublons)
        if [ -f "$LEAK_FILE" ]; then
            # Utiliser sort -u pour éliminer les doublons en une seule passe
            sort -u -t';' -k1,1 "$LEAK_FILE" > "${LEAK_FILE}.tmp"
            mv "${LEAK_FILE}.tmp" "$LEAK_FILE"
            
            # Vérifier que le fichier ne contient que des lignes au format usine;valeur
            if grep -v "^[^;]*;[^;]*$" "$LEAK_FILE" > /dev/null; then
                log_error "Le fichier leaks.dat contient des lignes au format incorrect."
                # Filtrer pour ne garder que les lignes valides
                grep "^[^;]*;[^;]*$" "$LEAK_FILE" > "${LEAK_FILE}.clean"
                mv "${LEAK_FILE}.clean" "$LEAK_FILE"
            fi
>>>>>>> origin/main
        fi
        ;;

    *)
<<<<<<< HEAD
        echo "Commande inconnue."
=======
        log_error "Commande inconnue ('$COMMAND'). Utilisez 'histo' ou 'leaks'."
>>>>>>> origin/main
        exit 1
        ;;
esac

# Fin du chronomètre global
GLOBAL_END=$(date +%s%3N)
TOTAL_DUREE=$((GLOBAL_END - GLOBAL_START))
<<<<<<< HEAD
echo "Durée totale du traitement : ${TOTAL_DUREE} ms"

exit 0
>>>>>>> origin/teuteu_test
=======

show_separator
log_success "Traitement terminé en ${BOLD}${TOTAL_DUREE} ms${RESET}"
echo "=== Fin du traitement $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$LOG_FILE"

exit 0
>>>>>>> origin/main
