#!/bin/bash

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
