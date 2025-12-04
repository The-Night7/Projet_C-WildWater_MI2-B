/**
 * @file histogram.c
 * @brief Implémentation des fonctions pour la génération d'histogrammes
 * @author Votre équipe
 * @date Décembre 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "histogram.h"
#include "histogram_modes.h"
#include "utils.h"

// Structure pour stocker les entrées triées pour l'histogramme
typedef struct {
    HistogramEntry* entries;
    int count;
    int capacity;
} SortedEntries;

// Fonction de comparaison pour le tri des entrées par valeur (décroissant)
static int compare_entries_desc(const void* a, const void* b) {
    const HistogramEntry* entry_a = (const HistogramEntry*)a;
    const HistogramEntry* entry_b = (const HistogramEntry*)b;
    
    // Tri par valeur décroissante
    if (entry_b->value > entry_a->value) return 1;
    if (entry_b->value < entry_a->value) return -1;
    
    // En cas d'égalité, tri par identifiant (ordre alphabétique inverse)
    return strcmp(entry_b->factory_id, entry_a->factory_id);
}

/**
 * @brief Extrait les données de l'AVL selon le mode spécifié
 * 
 * @param root Racine de l'arbre AVL
 * @param mode Mode d'histogramme
 * @return SortedEntries* Structure contenant les entrées triées
 */
static SortedEntries* extract_sorted_entries(AVLNode* root, HistogramMode mode) {
    // Compter le nombre de nœuds dans l'arbre
    int count = 0;
    void count_nodes(AVLNode* node) {
        if (node == NULL) return;
        count++;
        count_nodes(node->left);
        count_nodes(node->right);
    }
    count_nodes(root);
    
    // Allouer la structure et le tableau d'entrées
    SortedEntries* sorted = malloc(sizeof(SortedEntries));
    if (!sorted) return NULL;
    
    sorted->entries = malloc(count * sizeof(HistogramEntry));
    if (!sorted->entries) {
        free(sorted);
        return NULL;
    }
    
    sorted->count = 0;
    sorted->capacity = count;
    
    // Extraire les données de l'AVL
    void extract_data(AVLNode* node) {
        if (node == NULL) return;
        
        FactoryData* data = (FactoryData*)node->value;
        HistogramEntry* entry = &sorted->entries[sorted->count];
        
        entry->factory_id = strdup(node->key);
        
        // Sélectionner la valeur selon le mode
        switch (mode) {
            case HISTO_MODE_MAX:
                entry->value = data->capacity;
                break;
            case HISTO_MODE_SRC:
                entry->value = data->load_volume;
                break;
            case HISTO_MODE_REAL:
                entry->value = data->real_volume;
                break;
            default:
                entry->value = 0.0f;
        }
        
        sorted->count++;
        
        extract_data(node->left);
        extract_data(node->right);
    }
    extract_data(root);
    
    // Trier les entrées
    qsort(sorted->entries, sorted->count, sizeof(HistogramEntry), compare_entries_desc);
    
    return sorted;
}

/**
 * @brief Libère la mémoire utilisée par la structure SortedEntries
 * 
 * @param sorted Structure à libérer
 */
static void free_sorted_entries(SortedEntries* sorted) {
    if (!sorted) return;
    
    for (int i = 0; i < sorted->count; i++) {
        free(sorted->entries[i].factory_id);
    }
    
    free(sorted->entries);
    free(sorted);
}

/**
 * @brief Génère un fichier CSV contenant les données pour l'histogramme
 * 
 * @param filepath Chemin du fichier de sortie
 * @param root Racine de l'arbre AVL
 * @param mode Mode d'histogramme
 * @return int 0 en cas de succès, code d'erreur sinon
 */
int generate_histogram_csv(const char* filepath, AVLNode* root, HistogramMode mode) {
    if (!root) return 1;
    
    // Extraire et trier les données
    SortedEntries* sorted = extract_sorted_entries(root, mode);
    if (!sorted) return 2;
    
    // Ouvrir le fichier de sortie
    FILE* file = fopen(filepath, "w");
    if (!file) {
        free_sorted_entries(sorted);
        return 3;
    }
    
    // Écrire l'en-tête
    fprintf(file, "Station;%s\n", get_histogram_mode_description(mode));
    
    // Écrire les données
    for (int i = 0; i < sorted->count; i++) {
        fprintf(file, "%s;%.3f\n", sorted->entries[i].factory_id, sorted->entries[i].factory_id, sorted->entries[i].value);
    }
    
    fclose(file);
    
    // Libérer la mémoire
    free_sorted_entries(sorted);
    
    return 0;
}

/**
 * @brief Prépare les données pour la génération d'un histogramme PNG
 * 
 * @param filepath Chemin du fichier de sortie
 * @param root Racine de l'arbre AVL
 * @param mode Mode d'histogramme
 * @return int 0 en cas de succès, code d'erreur sinon
 */
int prepare_histogram_data(const char* filepath, AVLNode* root, HistogramMode mode) {
    if (!root) return 1;
    
    // Extraire et trier les données
    SortedEntries* sorted = extract_sorted_entries(root, mode);
    if (!sorted) return 2;
    
    // Ouvrir le fichier de données temporaire pour GnuPlot
    FILE* file = fopen(filepath, "w");
    if (!file) {
        free_sorted_entries(sorted);
        return 3;
    }
    
    // Écrire les 5 plus grandes valeurs
    fprintf(file, "# Les 5 plus grandes valeurs\n");
    int max_entries = sorted->count < 5 ? sorted->count : 5;
    for (int i = 0; i < max_entries; i++) {
        fprintf(file, "%s %.3f\n", sorted->entries[i].factory_id, sorted->entries[i].value);
    }
    
    fprintf(file, "\n\n");
    
    // Écrire les 5 plus petites valeurs
    fprintf(file, "# Les 5 plus petites valeurs\n");
    int start_idx = sorted->count > 5 ? sorted->count - 5 : 0;
    for (int i = start_idx; i < sorted->count; i++) {
        fprintf(file, "%s %.3f\n", sorted->entries[i].factory_id, sorted->entries[i].value);
    }
    
    fclose(file);
    
    // Libérer la mémoire
    free_sorted_entries(sorted);
    
    // Note: La génération du PNG sera faite par un script externe utilisant GnuPlot
    
    return 0;
}

/**
 * @brief Prépare les données pour l'histogramme cumulé (bonus)
 * 
 * @param filepath Chemin du fichier de sortie
 * @param root Racine de l'arbre AVL
 * @return int 0 en cas de succès, code d'erreur sinon
 */
int prepare_combined_histogram_data(const char* filepath, AVLNode* root) {
    if (!root) return 1;
    
    // Ouvrir le fichier de données temporaire pour GnuPlot
    FILE* file = fopen(filepath, "w");
    if (!file) return 2;
    
    // Écrire l'en-tête
    fprintf(file, "# Données pour histogramme cumulé\n");
    fprintf(file, "# Station Capacité Volume_Capté Volume_Réel\n");
    
    // Parcourir l'AVL et écrire les données
    void write_data(AVLNode* node) {
        if (node == NULL) return;
        
        write_data(node->left);
        
        FactoryData* data = (FactoryData*)node->value;
        fprintf(file, "%s %.3f %.3f %.3f\n", 
                node->key, 
                data->capacity, 
                data->load_volume, 
                data->real_volume);
        
        write_data(node->right);
    }
    write_data(root);
    
    fclose(file);
    
    return 0;
}