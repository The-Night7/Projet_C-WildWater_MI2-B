//
// Created by myria on 12/2/2025.
//

#ifndef PROJET_C_WILDWATER_MI2_B_HISTOGRAM_H
#define PROJET_C_WILDWATER_MI2_B_HISTOGRAM_H

/**
 * @brief Structure contenant les données agrégées pour une station/usine.
 * Cette structure sera stockée dans le champ 'void* value' du noeud AVL.
 */
typedef struct {
    float capacity;      // Capacité de l'usine (utilisé pour l'argument 'max')
    float load_volume;   // Volume total traité (utilisé pour l'argument 'src')
    float real_volume;   // Volume réel consommé (utilisé pour l'argument 'real')
    int count;           // Compteur d'occurrences (utile pour debug ou moyenne)
} FactoryData;

#endif //PROJET_C_WILDWATER_MI2_B_HISTOGRAM_H