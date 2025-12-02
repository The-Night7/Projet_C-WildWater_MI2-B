#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

int main(int argc, char** argv) {
    if (argc != 3) return 1;

    FILE* file = fopen(argv[1], "r");
    if (!file) return 2;

    char* mode = argv[2];
    Station* root = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), file)) {
        char *col1, *col2, *col3, *col4, *col5;
        
        col1 = strtok(line, ",");
        col2 = strtok(NULL, ",");
        col3 = strtok(NULL, ",");
        col4 = strtok(NULL, ",");
        col5 = strtok(NULL, ",");

        // Suppression du warning "variable set but not used"
        (void)col1; 

        if (!col4) continue;

        // Mode MAX
        if (strcmp(mode, "max") == 0) {
            if (col2 && strstr(col2, "Plant") && col3 && (strcmp(col3, "-") == 0 || strcmp(col3, " ") == 0)) {
                root = insert_station(root, col2, atol(col4), 0, 0);
            }
        }
        // Mode SRC / REAL
        else if (col3 && strstr(col3, "Plant")) {
            long vol = atol(col4);
            long reel = vol;
            if (strcmp(mode, "real") == 0 && col5) {
                float p = atof(col5);
                reel = (long)(vol * (1.0 - (p/100.0)));
            }
            root = insert_station(root, col3, 0, vol, reel);
        }
    }

    fclose(file);
    write_csv(root, stdout, mode);
    free_tree(root);

    return 0;
}