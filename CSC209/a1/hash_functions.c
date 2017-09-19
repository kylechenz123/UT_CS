#include <stdio.h>

// Complete these two functions according to the assignment specifications


void hash(char *hash_val, long block_size) {
    int i;
    for (i = 0; i < block_size; i++){
      hash_val[i] = '\0';
    }
    char user_input;
    long x = 0;
    while(scanf("%c\n", &user_input) != EOF){
        hash_val[x % block_size] = hash_val[x % block_size] ^ user_input;
        x += 1;
    }
}


int check_hash(const char *hash1, const char *hash2, long block_size) {

    // A place holder so that the file will compile without error.
	// Replace with the correct return value.
    int i;
    for (i = 0; i < block_size; i++){
        if(hash1[i] != hash2[i]){
          return i;
        }
    }
    return i;
}
