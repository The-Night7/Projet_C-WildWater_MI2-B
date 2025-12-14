/*
 * main.c
 *
 * Point d'entrée du programme C‑WildWater.  Ce fichier lit un fichier de
 * données représentant le réseau d'eau potable, agrège les informations
 * demandées pour produire des histogrammes ou calcule les pertes en aval
 * d'une usine.  L'algorithme de parcours utilise un arbre AVL pour
 * stocker les usines triées et un graphe orienté pour représenter les
 * relations entre elles.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

/*
 * Supprime les espaces blancs en début et en fin de chaîne.  Renvoie
 * un pointeur vers le premier caractère non blanc de l’argument et
 * tronque la fin de la chaîne en place.  Utilisé pour nettoyer les
 * champs lus du fichier CSV afin d’éviter des identifiants
 * différents à cause d’espaces superflus.
 */
static char* trim_whitespace(char* str) {
    if (!str) return NULL;
    /* Avancer jusqu’au premier caractère non blanc */
    while (*str && (*str == ' ' || *str == '\t')) {
        str++;
    }
    if (*str == '\0') return str;
    /* Reculer depuis la fin pour supprimer les blancs */
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) {
        *end = '\0';
        end--;
    }
    return str;
}

// Définition pour le mode "all"
#define ALL_LEAKS

// -----------------------------------------------------------------------------
//  Fonction récursive de calcul des fuites (DFS)
//
// À partir d'un nœud `node` et d'un volume d'entrée `input_vol`, calcule
// récursivement la quantité totale d'eau perdue en aval.  On répartit le
// volume d'entrée équitablement entre tous les enfants, on applique le
// pourcentage de fuite du tronçon et on additionne les pertes locales et
// celles des sous‑arbres.
static double solve_leaks(Station* node, double input_vol) {
    if (!node || node->nb_children == 0) {
        return 0.0;
    }
    double total_loss = 0.0;
    double vol_per_pipe = input_vol / node->nb_children;
    AdjNode* curr = node->children;
    while (curr) {
        // Perte sur ce tronçon
        double pipe_loss = 0.0;
        if (curr->leak_perc > 0) {
            pipe_loss = vol_per_pipe * (curr->leak_perc / 100.0);
        }
        // Ce qui arrive réellement à l'enfant
        double vol_arrived = vol_per_pipe - pipe_loss;
        // Somme : perte locale + pertes des enfants
        total_loss += pipe_loss + solve_leaks(curr->target, vol_arrived);
        curr = curr->next;
    }
    return total_loss;
}

// -----------------------------------------------------------------------------
// Fonction pour calculer les fuites de toutes les usines
//
// Cette fonction parcourt l'arbre AVL et calcule les fuites pour chaque usine
// qui a une capacité ou un volume réel entrant. Les résultats sont écrits
// dans un fichier au format "nom_usine;valeur_fuite".
static void calculate_all_leaks(Station* node, FILE* output) {
    if (!node) return;

    // Traitement récursif (parcours infixe)
    calculate_all_leaks(node->left, output);

    /*
     * Calcul des fuites pour cette station.  Le volume de départ doit
     * correspondre au volume d'eau réellement traité par l'usine (real_qty)
     * et non à la capacité théorique.  Si une station ne possède aucun
     * volume réel entrant, elle est ignorée car elle ne participe pas au
     * calcul des fuites (cas des nœuds non-usines comme les jonctions ou
     * espaces de stockage).  La capacité n'est utilisée que pour le mode
     * histogramme, pas pour l'évaluation des rendements.
     */
    double starting_volume = (double)node->real_qty;

    if (starting_volume > 0) {
        double leaks = solve_leaks(node, starting_volume);
        /* Conversion en millions de m³ (division par 1000) */
        fprintf(output, "%s;%.6f\n", node->name, leaks / 1000.0);
    }

    calculate_all_leaks(node->right, output);
}

// -----------------------------------------------------------------------------
// Fonction pour compter le nombre d'usines dans l'arbre
static int count_stations(Station* node) {
    if (!node) return 0;
    return 1 + count_stations(node->left) + count_stations(node->right);
}

// -----------------------------------------------------------------------------
//  Main
//
// Le programme attend deux ou trois arguments sur la ligne de commande :
//  * argv[1] : chemin vers le fichier de données (.dat ou .csv)
//  * argv[2] : mode d'histogramme (« max », « src » ou « real »), ou
//              identifiant d'une usine pour le calcul des fuites, ou
//              "all" pour calculer les fuites de toutes les usines
//
// En mode histogramme, un fichier CSV est écrit sur la sortie standard.
// En mode fuites, un nombre (double) représentant la perte en millions de
// mètres cubes est affiché.
// En mode "all", un fichier CSV avec toutes les usines et leurs fuites est généré.
int main(int argc, char** argv) {
    if (argc != 3) {
        return 1; // Code de retour > 0 si les arguments sont incorrects
    }
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        return 2; // Impossible d'ouvrir le fichier d'entrée
    }

    // --- OPTIMISATION I/O (Ajout pour WSL) ---
    const size_t BUF_SIZE = 16 * 1024 * 1024; // 16 Mo
    char* big_buffer = malloc(BUF_SIZE);
    if (big_buffer) setvbuf(file, big_buffer, _IOFBF, BUF_SIZE);
    // -----------------------------------------

    char* arg_mode = argv[2];
    int mode_histo = 0; // 1 = max, 2 = src, 3 = real
    int mode_leaks = 0;
    int mode_all_leaks = 0;

    if (strcmp(arg_mode, "max") == 0) {
        mode_histo = 1;
    } else if (strcmp(arg_mode, "src") == 0) {
        mode_histo = 2;
    } else if (strcmp(arg_mode, "real") == 0) {
        mode_histo = 3;
    } else if (strcmp(arg_mode, "all") == 0) {
        mode_all_leaks = 1;
    } else {
        mode_leaks = 1; // Tout autre argument est considéré comme un identifiant d'usine
    }

    Station* root = NULL;
    char line[1024];
    long line_count = 0;
    long station_count = 0;
    long capacity_count = 0;

    // Lecture ligne par ligne du fichier d'entrée
    while (fgets(line, sizeof(line), file)) {
        line_count++;
        // Affichage sur stderr toutes les 200 000 lignes pour suivre la progression
        if (line_count % 200000 == 0) {
            /*
             * Affichage périodique de la progression.  On imprime sur la
             * sortie d'erreur le nombre de lignes traitées toutes les
             * 200 000 lignes.  L'utilisation d'\r permet de réécrire la
             * même ligne dans le terminal.
             */
            fprintf(stderr, "Lignes traitees : %ld...\r", line_count);
            fflush(stderr);
        }
        // Nettoyage des retours chariot
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) < 2) {
            continue;
        }
        // Découpage manuel de la ligne par points‑virgules
        char* cols[5] = {NULL};
        char* p = line;
        int c = 0;
        cols[c++] = p;
        while (*p && c < 5) {
            if (*p == ';') {
                *p = '\0';
                cols[c++] = p + 1;
            }
            p++;
        }
        /*
         * Nettoyage des champs : suppression des espaces superflus et
         * remplacement des tirets ou chaînes vides par NULL.  Cela évite
         * de créer plusieurs nœuds avec des identifiants identiques mais
         * différemment espacés, et permet de reconnaître les valeurs
         * manquantes indiquées par un tiret.
         */
        for (int i = 0; i < 5; i++) {
            if (cols[i]) {
                cols[i] = trim_whitespace(cols[i]);
                if (strcmp(cols[i], "-") == 0 || strlen(cols[i]) == 0) {
                    cols[i] = NULL;
                }
            }
        }
        // -----------------------------------------------------------------
        // Mode « leaks » ou « all » : construire le graphe et l'arbre des stations
        // -----------------------------------------------------------------
        if (mode_leaks || mode_all_leaks) {
            // Associer les identifiants (colonne 2 et 3) à des stations
            if (cols[1]) {
                Station* temp = find_station(root, cols[1]); // Optimisation recherche
                if (!temp) {
                    root = insert_station(root, cols[1], 0, 0, 0);
                    station_count++;
                }
            }
            if (cols[2]) {
                Station* temp = find_station(root, cols[2]); // Optimisation recherche
                if (!temp) {
                    root = insert_station(root, cols[2], 0, 0, 0);
                    station_count++;
                }
            }
            /*
             * Création de la connexion amont→aval.  Dans le fichier CSV, la
             * colonne #2 (index 1) contient l’identifiant de l’acteur amont
             * (usine, stockage, jonction, raccordement, etc.) et la colonne #3
             * (index 2) l’identifiant de l’acteur aval.  S’il manque l’une des
             * deux colonnes, aucune connexion n’est ajoutée.  La colonne #5
             * (index 4) peut contenir un pourcentage de fuites qui sera
             * stocké dans le graphe.
             */
            if (cols[1] && cols[2]) {
                Station* pa = find_station(root, cols[1]);
                Station* ch = find_station(root, cols[2]);
                double leak = (cols[4] ? atof(cols[4]) : 0.0);
                add_connection(pa, ch, leak);

                /*
                 * Mise à jour du volume réel entrant pour l’acteur aval.
                 * La colonne #4 (index 3) peut contenir un volume sortant
                 * (uniquement dans les lignes SOURCE→USINE).  Ce volume est
                 * exprimé en milliers de m³.  Pour calculer le volume réel
                 * qui arrive effectivement à l’aval, on applique le
                 * pourcentage de fuite indiqué dans la colonne #5.  Les
                 * volumes sont cumulés dans le champ real_qty de l’acteur
                 * aval si nécessaire.
                 */
                /*
                 * Mise à jour du volume réel entrant : ce volume (colonne #4) ne
                 * doit être pris en compte que pour les tronçons « source →
                 * usine ».  Dans ces lignes, la colonne #1 est vide (notée
                 * '-' dans le fichier CSV, transformé en NULL après
                 * nettoyage).  Pour les autres types de tronçons (usine→
                 * stockage, stockage→jonction, etc.), la colonne #4 ne
                 * représente pas un volume capté et ne doit donc pas être
                 * utilisée pour mettre à jour real_qty.  De cette manière,
                 * seules les usines reçoivent un volume réel.
                 */
                if (cols[3] && !cols[0]) {
                    double vol = atof(cols[3]);
                    double real_vol = vol * (1.0 - leak / 100.0);
                    if (mode_all_leaks && ch) {
                        ch->real_qty += (long)real_vol;
                    } else if (mode_leaks && ch && strcmp(ch->name, arg_mode) == 0) {
                        ch->real_qty += (long)real_vol;
                    }
                }
            }

            /*
             * Mise à jour de la capacité de l'usine (colonne #4) si la
             * colonne #3 est vide.  Dans le format du fichier, une ligne
             * « USINE » a son identifiant en colonne #2 et sa capacité
             * en colonne #4.  Aucune connexion n’est créée pour ces lignes.
             * On cumule les capacités au cas où une même usine
             * apparaîtrait plusieurs fois.
             */
            if (cols[1] && !cols[2] && cols[3]) {
                Station* s = find_station(root, cols[1]);
                if (s) {
                    s->capacity += atol(cols[3]);
                    capacity_count++;
                }
            }
        } else {
            // -----------------------------------------------------------------
            // Mode histogramme : agrégation selon le mode choisi
            // -----------------------------------------------------------------
            // Cas 1 : mode « max » : on récupère toutes les définitions d'usine
            // Structure: [Ignoré];[ID Usine];[Vide];[Capacité];[Ignoré]
            if (mode_histo == 1 && cols[1] && !cols[2] && cols[3]) {
                root = insert_station(root, cols[1], atol(cols[3]), 0, 0);
            }
            // Cas 2 : modes « src » et « real » : on agrège les volumes des sources
            // Structure: [Ignoré];[ID Source];[ID Usine];[Volume];[Fuite]
            else if ((mode_histo == 2 || mode_histo == 3) && cols[2] && cols[3]) {
                long vol = atol(cols[3]);
                long reel = vol;
                if (mode_histo == 3 && cols[4]) {
                    double p_leak = atof(cols[4]);
                    reel = (long)(vol * (1.0 - (p_leak / 100.0)));
                }
                root = insert_station(root, cols[2], 0, vol, reel);
            }
        }
    }

    // Afficher des statistiques en mode "all"
    if (mode_all_leaks) {
        fprintf(stderr, "Lignes traitées: %ld\n", line_count);
        fprintf(stderr, "Stations créées: %ld\n", station_count);
        fprintf(stderr, "Capacités définies: %ld\n", capacity_count);
        fprintf(stderr, "Nombre total de stations dans l'AVL: %d\n", count_stations(root));
    }
    fclose(file);

    // Sorties finales
    if (mode_leaks) {
        Station* start = find_station(root, arg_mode);
        if (!start) {
            /*
             * Usine introuvable : renvoyer -1.  Cette convention est
             * utilisée par le script pour distinguer une absence de
             * résultat d'une usine ayant effectivement 0 M.m3 de pertes.
             */
            printf("-1\n");
        } else {
            /*
             * Choix du volume de départ : on utilise le volume réellement
             * traité par l'usine (real_qty) et on ignore la capacité si
             * aucune eau n'a été captée.  Conformément au cahier des
             * charges, le calcul des fuites doit se baser sur le volume
             * effectivement en sortie de l'usine.  La capacité ne sert
             * que dans les histogrammes.
             */
            double starting_volume = (double)start->real_qty;
            double leaks = 0.0;
            if (starting_volume > 0) {
                leaks = solve_leaks(start, starting_volume);
            }
            /* Conversion en millions de m³ */
            printf("%.6f\n", leaks / 1000.0);
        }
    } else if (mode_all_leaks) {
        // Mode "all" : calculer les fuites pour toutes les usines
        calculate_all_leaks(root, stdout);
    } else {
        // Mode histogramme : écrire le CSV sur stdout
        char mode_str[10];
        if (mode_histo == 1) strcpy(mode_str, "max");
        if (mode_histo == 2) strcpy(mode_str, "src");
        if (mode_histo == 3) strcpy(mode_str, "real");
        write_csv(root, stdout, mode_str);
    }

    // Libération de la mémoire
    free_tree(root);
    if (big_buffer) free(big_buffer);

    return 0;
}