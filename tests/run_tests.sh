#!/bin/bash
#
# Script d'automatisation des tests pour C-WildWater
# Ce script exécute les tests sur différents jeux de données et compare les résultats
#
# Auteur : Votre équipe
# Date : Novembre 2025

# Définition des chemins
TEST_ROOT=$(dirname $(readlink -f "$0"))
PROJECT_ROOT=$(dirname "$TEST_ROOT")
C_PROGRAM="$TEST_ROOT/c-wildwater"
TEST_CASES="$TEST_ROOT/test_cases"
EXPECTED="$TEST_ROOT/expected_outputs"
LOGS="$TEST_ROOT/logs"

# Couleurs pour les messages
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
BLUE="\033[0;34m"
RESET="\033[0m"

# Compteurs pour les résultats des tests
passed=0
failed=0
total=0

# Fonction pour exécuter un test
run_test() {
    local test_name="$1"
    local test_command="$2"
    local expected_file="$3"
    local output_file="$4"
    local log_file="$LOGS/test_${test_name}.log"
    
    echo -e "${YELLOW}Exécution du test: ${test_name}${RESET}"
    
    # Exécuter la commande et enregistrer le résultat dans le fichier log
    eval "$test_command" > "$log_file" 2>&1
    
    # Vérifier le code de retour
    if [ $? -ne 0 ]; then
        echo -e "${RED}Test échoué: La commande a retourné une erreur${RESET}"
        ((failed++))
        return
    fi
    
    # Vérifier si le fichier de sortie existe
    if [ ! -f "$output_file" ]; then
        echo -e "${RED}Test échoué: Fichier de sortie non généré${RESET}"
        ((failed++))
        return
    fi
    
    # Comparer le résultat avec la sortie attendue
    diff -q "$output_file" "$expected_file" > /dev/null
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}Test réussi: Les résultats correspondent${RESET}"
        ((passed++))
    else
        echo -e "${RED}Test échoué: Les résultats diffèrent${RESET}"
        echo "Différences:"
        diff "$output_file" "$expected_file" | head -n 10
        ((failed++))
    fi
}

# Vérifier si le programme existe
if [ ! -f "$C_PROGRAM" ]; then
    echo -e "${RED}Erreur: Le programme de test n'existe pas.${RESET}"
    echo "Assurez-vous de copier l'exécutable dans le répertoire de test."
    exit 1
fi

# Créer les répertoires de logs si nécessaire
mkdir -p "$LOGS"

echo -e "${BLUE}Démarrage des tests pour C-WildWater${RESET}"
echo "==============================================="

# Test 1: Histogramme de capacité maximale
if [ -f "$TEST_CASES/test_input_1.csv" ] && [ -f "$EXPECTED/expected_vol_max.dat" ]; then
    ((total++))
    run_test "histo_max" "$C_PROGRAM $TEST_CASES/test_input_1.csv $TEST_CASES/test_input_1.csv histo max" \
             "$EXPECTED/expected_vol_max.dat" "$TEST_ROOT/vol_max.dat"
fi

# Test 2: Histogramme de volume capté
if [ -f "$TEST_CASES/test_input_1.csv" ] && [ -f "$EXPECTED/expected_vol_captation.txt" ]; then
    ((total++))
    run_test "histo_src" "$C_PROGRAM $TEST_CASES/test_input_1.csv $TEST_CASES/test_input_1.csv histo src" \
             "$EXPECTED/expected_vol_captation.txt" "$TEST_ROOT/vol_captation.txt"
fi

# Test 3: Histogramme de volume traité
if [ -f "$TEST_CASES/test_input_1.csv" ] && [ -f "$EXPECTED/expected_vol_traitement.tmp" ]; then
    ((total++))
    run_test "histo_real" "$C_PROGRAM $TEST_CASES/test_input_1.csv $TEST_CASES/test_input_1.csv histo real" \
             "$EXPECTED/expected_vol_traitement.tmp" "$TEST_ROOT/vol_traitement.tmp"
fi

# Test 4: Calcul des pertes d'eau
if [ -f "$TEST_CASES/test_input_2.csv" ] && [ -f "$EXPECTED/expected_leaks.dat" ]; then
    ((total++))
    run_test "leaks" "$C_PROGRAM $TEST_CASES/test_input_2.csv $TEST_CASES/test_input_2.csv leaks USINE001" \
             "$EXPECTED/expected_leaks.dat" "$TEST_ROOT/leaks_history.dat"
fi

# Afficher les résultats
echo "==============================================="
echo -e "${BLUE}Résultats des tests:${RESET}"
echo "Tests exécutés: $total"
echo -e "${GREEN}Tests réussis: $passed${RESET}"
echo -e "${RED}Tests échoués: $failed${RESET}"

# Code de sortie basé sur les résultats
if [ $failed -eq 0 ]; then
    echo -e "${GREEN}Tous les tests ont réussi!${RESET}"
    exit 0
else
    echo -e "${RED}Certains tests ont échoué.${RESET}"
    exit 1
fi