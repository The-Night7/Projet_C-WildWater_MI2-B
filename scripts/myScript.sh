#!/bin/bash
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