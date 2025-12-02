#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct Station {
    int id;
    char* name;         // C'est ce champ qui posait problème !
    long capacity;
    long consumption;
    long real_qty;
    
    int height;
    struct Station* left;
    struct Station* right;
} Station;

#endif