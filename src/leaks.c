/**
 * @file leaks.c
 * @brief Implémentation du module pour le calcul des pertes d'eau
 * @author Votre équipe
 * @date Novembre 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "leaks.h"
#include "utils.h"

// Structure pour représenter un nœud dans le réseau de distribution
typedef struct NetworkNode {
    char id[32];              // Identifiant du nœud
    double volume;            // Volume d'eau
    double leak_percentage;   // Pourcentage de fuite
    int num_children;         // Nombre de nœuds enfants
    struct NetworkNode** children; // Tableau de pointeurs vers les nœuds enfants
} NetworkNode;

// Structure pour représenter l'arbre du réseau de distribution
typedef struct {
    NetworkNode* root;  // Racine de l'arbre (l'usine)
} NetworkTree;

int calculate_leaks(const char* data_file_v0, const char* data_file_v3, const char* plant_id) {
    printf("Calcul des pertes pour l'usine %s à partir des fichiers %s et %s\n", 
           plant_id, data_file_v0, data_file_v3);
    
    // À COMPLÉTER : Implémentation du calcul des pertes
    // Cette fonction devra:
    // 1. Charger les données des fichiers
    // 2. Construire l'arbre de distribution pour l'usine spécifiée
    // 3. Calculer les pertes d'eau dans tout le réseau aval
    // 4. Enregistrer les résultats dans l'historique

    // Exemple de résultat (à remplacer par le vrai calcul)
    double total_leaks = 0.0; // en millions de m³
    
    // Ajouter le résultat à l'historique
    add_to_leaks_history(plant_id, total_leaks);
    
    // Afficher le résultat
    printf("Volume total de pertes pour l'usine %s : %.2f millions de m³\n", plant_id, total_leaks);
    
    return 0;
}

int add_to_leaks_history(const char* plant_id, double leak_volume) {
    FILE* history_file = fopen("../data/leaks_history.dat", "a");
    if (!history_file) {
        fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier d'historique des pertes\n");
        return 1;
    }
    
    // Obtenir la date et l'heure actuelles
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    // Écrire l'enregistrement
    fprintf(history_file, "%s,%s,%.6f\n", timestamp, plant_id, leak_volume);
    
    fclose(history_file);
    printf("Enregistrement ajouté à l'historique des pertes\n");
    return 0;
}

int find_max_leak_segment(const char* data_file_v0, const char* data_file_v3) {
    printf("Recherche du tronçon avec la plus grande perte d'eau (bonus)...\n");
    
    // À COMPLÉTER : Implémentation de la recherche du tronçon avec la plus grande perte
    // Cette fonction devra:
    // 1. Charger les données des fichiers
    // 2. Parcourir tous les tronçons pour trouver celui avec la plus grande perte absolue
    // 3. Afficher les résultats
    
    return 0;
}