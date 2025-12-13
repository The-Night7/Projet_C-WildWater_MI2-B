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

# Vérification du nombre minimal d'arguments
if [ "$#" -lt 2 ]; then
    usage
fi

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

# Vérification que le fichier de données existe
if [ ! -f "$DATAFILE" ]; then
    echo "Erreur: le fichier '$DATAFILE' n'existe pas."
    exit 1
fi

# Compilation du binaire si nécessaire
if [ ! -x "$EXEC_MAIN" ]; then
    echo "--- Compilation du programme ---"
    (cd "$SRC_DIR" && make clean && make) || {
        echo "Erreur: échec de la compilation."
        exit 1
    }
fi

# Exécution selon le mode
case "$COMMAND" in
    histo)
        # Vérification du paramètre d'histogramme
        case "$PARAM" in
            max|src|real)
                ;;
            *)
                echo "Erreur: le mode d'histogramme doit être 'max', 'src' ou 'real'."
                exit 1
                ;;
        esac
        echo "--- Mode Histogramme ($PARAM) ---"
        OUT_CSV="$DATA_DIR/vol_${PARAM}.csv"
        # Exécuter le programme et trier par valeur ascendante (colonne 2)
        "$EXEC_MAIN" "$DATAFILE" "$PARAM" | LC_ALL=C sort -t';' -k2,2g > "$OUT_CSV"
        if [ ! -s "$OUT_CSV" ]; then
            echo "Erreur: CSV vide."
            exit 1
        fi
        # Générer le graphique des 10 plus grandes valeurs
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
        # Générer le graphique des 50 plus petites valeurs
        GP_SMALL="$DATA_DIR/data_small.dat"
        head -n 50 "$OUT_CSV" > "$GP_SMALL"
        IMG_SMALL="$GRAPH_DIR/vol_${PARAM}_small.png"
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
        echo "Image Bottom 50: $IMG_SMALL"
        # Nettoyage des fichiers temporaires
        rm -f "$GP_BIG" "$GP_SMALL"
        ;;
    leaks)
        echo "--- Mode Fuites ($PARAM) ---"
        LEAK_FILE="$DATA_DIR/leaks.dat"
        CACHE_FILE="$DATA_DIR/.leaks_cache.dat"

        # Créer le fichier cache s'il n'existe pas
        touch "$CACHE_FILE"

        # Mode "all" pour calculer toutes les fuites
        if [ "$PARAM" = "all" ]; then
            echo "Calcul des fuites pour toutes les usines..."

            # Créer un fichier temporaire pour stocker la liste des usines
            FACILITIES_LIST="$DATA_DIR/.facilities_list.txt"

            # Extraire la liste des usines du fichier de données si elle n'existe pas déjà
            if [ ! -f "$FACILITIES_LIST" ] || [ ! -s "$FACILITIES_LIST" ]; then
                echo "Extraction de la liste des usines (peut prendre du temps)..."
                # Extraction des identifiants d'usines uniques (colonne 2 quand colonne 3 est vide)
                awk -F';' '$3 == "-" && $2 != "-" {print $2}' "$DATAFILE" | sort -u > "$FACILITIES_LIST"
                echo "Liste des usines extraite: $(wc -l < "$FACILITIES_LIST") usines trouvées."
            fi

            # Vérifier que la liste des usines n'est pas vide
            if [ ! -s "$FACILITIES_LIST" ]; then
                echo "Erreur: impossible d'extraire la liste des usines."
                exit 1
            fi

            # Compiler le programme C avec l'option ALL_LEAKS si nécessaire
            if ! grep -q "ALL_LEAKS" "$SRC_DIR/main.c" 2>/dev/null; then
                echo "Le programme C ne supporte pas le mode 'all'. Veuillez implémenter cette fonctionnalité."
                echo "Consultez les instructions pour ajouter la fonction 'all' avec AVL au programme C."
                exit 1
            fi

            # Exécuter le programme avec l'option "all"
            START_TIME=$SECONDS
            "$EXEC_MAIN" "$DATAFILE" "all" > "$LEAK_FILE.new"
            CALC_TIME=$((SECONDS - START_TIME))

            # Vérifier que le fichier de résultat n'est pas vide
            if [ ! -s "$LEAK_FILE.new" ]; then
                echo "Erreur: aucun résultat généré."
                exit 1
            fi

            # Mettre à jour le fichier de fuites et le cache
            mv "$LEAK_FILE.new" "$LEAK_FILE"
            cp "$LEAK_FILE" "$CACHE_FILE"

            echo "Calcul terminé en ${CALC_TIME}s. Résultats enregistrés dans $LEAK_FILE"
            echo "Nombre d'usines traitées: $(wc -l < "$LEAK_FILE")"

        # Vérifier si l'usine a déjà été calculée (cache)
        elif [ -f "$CACHE_FILE" ]; then
            CACHED_VAL=$(grep -F "$(echo "$PARAM" | sed 's/;/\\;/g')" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
            if [ -n "$CACHED_VAL" ]; then
                echo "Résultat en cache pour '$PARAM': $CACHED_VAL M.m3"
                # Mise à jour du fichier de résultats si nécessaire
                if ! grep -q "$(echo "$PARAM" | sed 's/;/\\;/g')" "$LEAK_FILE" 2>/dev/null; then
                    echo "$PARAM;$CACHED_VAL" >> "$LEAK_FILE"
                fi
                exit 0
            fi
        fi

        # Si plusieurs usines sont spécifiées (séparées par des virgules) et ce n'est pas "all"
        if [[ "$PARAM" == *","* ]]; then
            echo "Traitement par lot de plusieurs usines..."
            IFS=',' read -ra FACILITIES <<< "$PARAM"
            for FAC in "${FACILITIES[@]}"; do
                # Suppression des espaces avant/après
                FAC=$(echo "$FAC" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')

                # Vérifier dans le cache
                CACHED_VAL=$(grep -F "$(echo "$FAC" | sed 's/;/\\;/g')" "$CACHE_FILE" | tail -n1 | cut -d';' -f2)
                if [ -n "$CACHED_VAL" ]; then
                    echo "[$FAC] Résultat en cache: $CACHED_VAL M.m3"
                    # Mise à jour du fichier de résultats si nécessaire
                    if ! grep -q "$(echo "$FAC" | sed 's/;/\\;/g')" "$LEAK_FILE" 2>/dev/null; then
                        echo "$FAC;$CACHED_VAL" >> "$LEAK_FILE"
                    fi
                    continue
                fi

                # Calcul des fuites pour cette usine
                START_TIME=$SECONDS
                VAL=$("$EXEC_MAIN" "$DATAFILE" "$FAC")
                CALC_TIME=$((SECONDS - START_TIME))

                # Traitement du résultat
                if [ "$VAL" = "-1" ]; then
                    echo "[$FAC] Usine introuvable (${CALC_TIME}s)"
                    echo "$FAC;-1" >> "$LEAK_FILE"
                    echo "$FAC;-1" >> "$CACHE_FILE"
                else
                    echo "[$FAC] Fuites: $VAL M.m3 (calculé en ${CALC_TIME}s)"
                    echo "$FAC;$VAL" >> "$LEAK_FILE"
                    echo "$FAC;$VAL" >> "$CACHE_FILE"
                fi
            done
        # Traitement d'une seule usine (si ce n'est pas "all")
        elif [ "$PARAM" != "all" ]; then
            # Traitement d'une seule usine
            START_TIME=$SECONDS
            VAL=$("$EXEC_MAIN" "$DATAFILE" "$PARAM")
            CALC_TIME=$((SECONDS - START_TIME))

            # Si le programme C renvoie -1, l'usine est introuvable.  Dans
            # tous les autres cas, la valeur représente le volume de pertes en
            # millions de m³.
            if [ "$VAL" = "-1" ]; then
                echo "Usine introuvable (recherche en ${CALC_TIME}s)."
                echo "$PARAM;-1" >> "$LEAK_FILE"
                echo "$PARAM;-1" >> "$CACHE_FILE"
            else
                echo "Fuites: $VAL M.m3 (calculé en ${CALC_TIME}s)"
                echo "$PARAM;$VAL" >> "$LEAK_FILE"
                echo "$PARAM;$VAL" >> "$CACHE_FILE"
            fi
        fi

        # Optimisation du fichier de fuites (élimination des doublons)
        if [ -f "$LEAK_FILE" ]; then
            sort -u -t';' -k1,1 "$LEAK_FILE" > "${LEAK_FILE}.tmp"
            mv "${LEAK_FILE}.tmp" "$LEAK_FILE"
        fi
        ;;
    *)
        echo "Erreur: commande inconnue ('$COMMAND').  Utilisez 'histo' ou 'leaks'."
        exit 1
        ;;
esac

echo "Durée du traitement : ${SECONDS}s"

exit 0
