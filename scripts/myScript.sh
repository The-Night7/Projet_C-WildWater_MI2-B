#!/bin/bash
#
# Script principal pour le projet C-WildWater
# Ce script orchestre l'exécution du programme C et gère les différentes fonctionnalités
#
# Auteur : Votre équipe
# Date : Novembre 2025

# Définition des chemins
PROJECT_ROOT=$(dirname $(dirname $(readlink -f "$0")))
SRC_DIR="${PROJECT_ROOT}/src"
DATA_DIR="${PROJECT_ROOT}/data"
C_PROGRAM="${SRC_DIR}/c-wildwater"

# Couleurs pour les messages
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
BLUE="\033[0;34m"
RESET="\033[0m"

# Fonction d'affichage de l'aide
display_help() {
    echo -e "${BLUE}C-WildWater - Analyse de données sur l'eau${RESET}"
    echo
    echo "Usage: $0 <commande> [options]"
    echo
    echo "Commandes disponibles :"
    echo "  histo max        : Génère un histogramme basé sur la capacité maximale des usines"
    echo "  histo src        : Génère un histogramme basé sur le volume d'eau capté"
    echo "  histo real       : Génère un histogramme basé sur le volume d'eau réellement traité"
    echo "  histo all        : Génère un histogramme cumulé des trois volumes (bonus)"
    echo "  leaks <id_usine> : Calcule les pertes d'eau pour une usine spécifique"
    echo
    echo "Options additionnelles :"
    echo "  -h, --help       : Affiche cette aide"
    echo "  -v, --verbose    : Mode verbeux (plus de détails)"
    echo
}

# Fonction de vérification de l'environnement
check_environment() {
    # Vérifier si le programme C existe
    if [ ! -f "$C_PROGRAM" ]; then
        echo -e "${RED}Erreur: Le programme C n'existe pas.${RESET}"
        echo -e "Essai de compilation..."
        
        # Essayer de compiler le programme
        cd "$SRC_DIR"
        make clean && make
        
        # Vérifier si la compilation a réussi
        if [ ! -f "$C_PROGRAM" ]; then
            echo -e "${RED}Erreur: La compilation a échoué.${RESET}"
            exit 1
        else
            echo -e "${GREEN}Compilation réussie.${RESET}"
        fi
    fi
    
    # Vérifier si les fichiers de données existent
    DATA_FILE_V0="${DATA_DIR}/c-wildwater_v0.dat"
    DATA_FILE_V3="${DATA_DIR}/c-wildwater_v3.dat"
    
    if [ ! -f "$DATA_FILE_V0" ] || [ ! -f "$DATA_FILE_V3" ]; then
        echo -e "${RED}Erreur: Fichiers de données manquants.${RESET}"
        echo "Vérifiez que les fichiers suivants existent :"
        echo "- $DATA_FILE_V0"
        echo "- $DATA_FILE_V3"
        exit 1
    fi
    
    # Créer les répertoires nécessaires s'ils n'existent pas
    mkdir -p "${DATA_DIR}/output_images"
}

# Fonction principale
main() {
    # Mesurer le temps d'exécution
    start_time=$(date +%s)
    
    # Vérifier si des arguments sont fournis
    if [ $# -eq 0 ]; then
        echo -e "${RED}Erreur: Aucun argument fourni.${RESET}"
        display_help
        exit 1
    fi
    
    # Traiter les options d'aide
    if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
        display_help
        exit 0
    fi
    
    # Vérifier l'environnement
    check_environment
    
    # Récupérer les fichiers de données
    DATA_FILE_V0="${DATA_DIR}/c-wildwater_v0.dat"
    DATA_FILE_V3="${DATA_DIR}/c-wildwater_v3.dat"
    
    # Traiter les commandes
    case "$1" in
        "histo")
            if [ $# -lt 2 ]; then
                echo -e "${RED}Erreur: Type d'histogramme non spécifié.${RESET}"
                display_help
                exit 1
            fi
            
            case "$2" in
                "max"|"src"|"real"|"all")
                    echo -e "${YELLOW}Génération d'histogramme ($2)...${RESET}"
                    "$C_PROGRAM" "$DATA_FILE_V0" "$DATA_FILE_V3" "histo" "$2"
                    exit_code=$?
                    ;;
                *)
                    echo -e "${RED}Erreur: Type d'histogramme invalide: $2${RESET}"
                    display_help
                    exit 1
                    ;;
            esac
            ;;
            
        "leaks")
            if [ $# -lt 2 ]; then
                echo -e "${RED}Erreur: Identifiant d'usine non spécifié.${RESET}"
                display_help
                exit 1
            fi
            
            echo -e "${YELLOW}Calcul des pertes pour l'usine $2...${RESET}"
            "$C_PROGRAM" "$DATA_FILE_V0" "$DATA_FILE_V3" "leaks" "$2"
            exit_code=$?
            ;;
            
        *)
            echo -e "${RED}Erreur: Commande inconnue: $1${RESET}"
            display_help
            exit 1
            ;;
    esac
    
    # Vérifier le code de sortie
    if [ $exit_code -ne 0 ]; then
        echo -e "${RED}Erreur: Le programme a échoué avec le code $exit_code.${RESET}"
        exit $exit_code
    else
        echo -e "${GREEN}Opération terminée avec succès.${RESET}"
    fi
    
    # Afficher le temps d'exécution
    end_time=$(date +%s)
    execution_time=$((end_time - start_time))
    echo -e "${BLUE}Temps d'exécution: ${execution_time} secondes.${RESET}"
}

# Exécuter la fonction principale avec tous les arguments
main "$@"