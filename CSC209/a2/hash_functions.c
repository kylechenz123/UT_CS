#include <stdio.h>
#include <stdlib.h>
#include "hash.h"

char *hash(FILE *f) {
    char *hash_val = malloc(BLOCK_SIZE * sizeof(char));

    if(hash_val == NULL){
      fprintf(stderr, "Unable to allocate memory for hash value.");
      exit(1);
    }

    int i;
    for (i = 0; i < BLOCK_SIZE; i++){
        hash_val[i] = '\0';
    }

    int x = 0;
    char input;
    while(fread(&input, sizeof(char), 1, f) != 0){
        hash_val[x] = hash_val[x] ^ input;
        x += 1;
        if(x == BLOCK_SIZE){
          x = 0;
        }
    }

    return hash_val;
}
