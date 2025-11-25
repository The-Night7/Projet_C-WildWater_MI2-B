/**
 * @file histogram.c
 * @brief Implémentation du module pour la génération des histogrammes
 * @author Votre équipe
 * @date Novembre 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "histogram.h"
#include "utils.h"

// Structure pour stocker les données d'une usine pour les histogrammes
typedef struct {
    char plant_id[32];  // Identifiant de l'usine
    double max_capacity; // Capacité maximale
    double src_volume;   // Volume capté
    double real_volume;  // Volume réellement traité
} PlantData;

int generate_histogram(const char* data_file_v0, const char* data_file_v3, int histo_type) {
    printf("Génération de l'histogramme (type %d) à partir des fichiers %s et %s\n", 
           histo_type, data_file_v0, data_file_v3);

    // Charger et traiter les données ici
    // Cette fonction devra implémenter l'algorithme complet pour:
    // 1. Charger les données des fichiers
    // 2. Construire la structure de données appropriée (AVL mentionné dans les spécifications)
    // 3. Calculer les valeurs requises selon le type d'histogramme
    // 4. Trier les résultats par identifiant d'usine (ordre alphabétique inverse)
    // 5. Sauvegarder les résultats dans les fichiers appropriés
    // 6. Générer l'image de l'histogramme

    const char* output_file = NULL;
    const char* output_image = NULL;
    const char* title = NULL;

    // Déterminer le fichier de sortie et le titre en fonction du type d'histogramme
    switch (histo_type) {
        case HISTO_TYPE_MAX:
            output_file = "../data/vol_max.dat";
            output_image = "../data/output_images/vol_max.png";
            title = "Capacité maximale des usines";
            break;
        case HISTO_TYPE_SRC:
            output_file = "../data/vol_captation.txt";
            output_image = "../data/output_images/vol_captation.png";
            title = "Volume d'eau capté par usine";
            break;
        case HISTO_TYPE_REAL:
            output_file = "../data/vol_traitement.tmp";
            output_image = "../data/output_images/vol_traitement.png";
            title = "Volume d'eau réellement traité par usine";
            break;
        case HISTO_TYPE_ALL:
            output_file = "../data/vol_all.dat";
            output_image = "../data/output_images/vol_all.png";
            title = "Volumes comparatifs par usine";
            break;
        default:
            fprintf(stderr, "Type d'histogramme non reconnu: %d\n", histo_type);
            return 1;
    }

    // Créer le répertoire de sortie des images s'il n'existe pas
    system("mkdir -p ../data/output_images");

    // À COMPLÉTER : Implémentation de la génération d'histogramme
    
    // Exemple de génération d'image (à adapter avec les vraies données)
    return generate_histogram_image(output_file, output_image, title, histo_type);
}

int save_histogram_results(void* results, int count, const char* output_file) {
    // À COMPLÉTER : Sauvegarde des résultats dans le fichier de sortie
    // Cette fonction devra écrire les résultats dans le format approprié
    
    printf("Sauvegarde des résultats dans %s\n", output_file);
    return 0;
}

int generate_histogram_image(const char* data_file, const char* output_image, const char* title, int histo_type) {
    // Utiliser GnuPlot pour générer l'image
    // Cette fonction devra créer un script GnuPlot temporaire et l'exécuter
    
    // Exemple de script GnuPlot (à adapter selon le type d'histogramme)
    FILE* gnuplot_script = fopen("temp_gnuplot.plt", "w");
    if (!gnuplot_script) {
        fprintf(stderr, "Erreur lors de la création du script GnuPlot\n");
        return 1;
    }

    fprintf(gnuplot_script, "set terminal png size 800,600\n");
    fprintf(gnuplot_script, "set output '%s'\n", output_image);
    fprintf(gnuplot_script, "set title '%s'\n", title);
    fprintf(gnuplot_script, "set xlabel 'Usines'\n");
    fprintf(gnuplot_script, "set ylabel 'Volume (milliers de m³)'\n");
    
    if (histo_type == HISTO_TYPE_ALL) {
        // Pour l'histogramme cumulé (bonus)
        fprintf(gnuplot_script, "set style data histograms\n");
        fprintf(gnuplot_script, "set style histogram rowstacked\n");
        fprintf(gnuplot_script, "set boxwidth 0.7\n");
        fprintf(gnuplot_script, "set style fill solid 1.0 border -1\n");
        fprintf(gnuplot_script, "plot '%s' using 2:xtic(1) title 'Capacité max', '' using 3 title 'Volume capté', '' using 4 title 'Volume traité'\n", data_file);
    } else {
        // Pour les autres types d'histogrammes
        fprintf(gnuplot_script, "set style data histograms\n");
        fprintf(gnuplot_script, "set style fill solid 0.7\n");
        fprintf(gnuplot_script, "set xtics rotate by -45\n");
        fprintf(gnuplot_script, "plot '%s' using 2:xtic(1) title ''\n", data_file);
    }
    
    fclose(gnuplot_script);
    
    // Exécuter GnuPlot
    system("gnuplot temp_gnuplot.plt");
    system("rm temp_gnuplot.plt");
    
    printf("Image générée: %s\n", output_image);
    return 0;
}