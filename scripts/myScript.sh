#!/bin/bash
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
EOF
    exit 1
}

# Vérification du nombre minimal d'arguments
if [ "$#" -lt 2 ]; then
    usage
fi

# Gestion des arguments (Fichier optionnel)
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

        # --- BRANCHEMENT : MODE BONUS "ALL" ou MODES CLASSIQUES ---
        if [ "$PARAM" = "all" ]; then
            # ---------------------------------------------------------
            # BONUS 1 : Histogramme combiné (Capacité / Source / Réel)
            # ---------------------------------------------------------
            IMG_ALL="$GRAPH_DIR/vol_all.png"
            GP_DATA="$DATA_DIR/data_all.dat"

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

        else
            # ---------------------------------------------------------
            # MODES CLASSIQUES (max, src, real)
            # ---------------------------------------------------------
            
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
            echo "Image Top 10: $IMG_BIG"

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
                set ylabel 'Volume'
                set yrange [*:*]
                set xtics rotate by -90 font ',8'
                plot '$GP_SMALL' using 2:xtic(1) title 'Volume' lc rgb 'red'
EOF
            echo "Image Bottom 50: $IMG_SMALL"
            rm -f "$GP_BIG" "$GP_SMALL"
        fi
        ;;

    leaks)
        # ---------------------------------------------------------
        # MODE LEAKS (Calcul de fuites)
        # ---------------------------------------------------------
        echo "--- Mode Fuites ($PARAM) ---"
        LEAK_FILE="$DATA_DIR/leaks.dat"
        CACHE_FILE="$DATA_DIR/.leaks_cache.dat"
        touch "$CACHE_FILE"

        # Fonction interne pour traiter une usine
        process_factory() {
            local FACTORY="$1"
            # Nettoyage espaces
            FACTORY=$(echo "$FACTORY" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')

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
        fi
        ;;

    *)
        echo "Commande inconnue."
        exit 1
        ;;
esac

# Fin du chronomètre global
GLOBAL_END=$(date +%s%3N)
TOTAL_DUREE=$((GLOBAL_END - GLOBAL_START))
echo "Durée totale du traitement : ${TOTAL_DUREE} ms"

exit 0
>>>>>>> origin/teuteu_test
