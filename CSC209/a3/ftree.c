#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <sys/wait.h>
#include "ftree.h"
#include "hash.h"

/* Returns a new path by combining the current path
*  and the given filename with a slash as the separator in the middle.
*/
char *generate_new_path(const char * current_path, char *fname){
    char *separator = "/";
    int space_needed = strlen(separator) + strlen(current_path)
                       + strlen(fname) + 1;

    char* new_path = malloc(space_needed);

    if(new_path == NULL){
        fprintf(stderr, "Unable to allocate memory for new path.");
        exit(-1);
    }

    strncpy(new_path, current_path, strlen(current_path) + 1);
    strncat(new_path, separator, strlen(separator));
    strncat(new_path, fname, strlen(fname));

    return new_path;
}

/* Copy the source file located at source_path to the destination directory 
*  located at dest_path. After the copying process, return the number of 
*  processes used by this program. 
*/   
int copy_ftree(const char *source_path, const char *dest_path) {
	 // The accumulator for number of processes used.
	 int return_value = 1;
	
    struct stat source_info, dest_info;

    int source_error = lstat(source_path, &source_info);
    int dest_error = lstat(dest_path, &dest_info);
    
    if(source_error != 0){
        perror("lstat for source");
        return -1;
    }
    
    if(dest_error != 0){
        perror("lstat for destination");
        return -1;
    }
    
    // The destination must be a directory.
    // Also need avoid copying a file to itself. 
    if(S_ISDIR(dest_info.st_mode) != 1 || strcmp(source_path, dest_path) == 0) {
    	  fprintf(stderr, "Destination is not a directory or destination is the same as the source file. \n");
        return -1;    
    }
    	
    char *source_filename = basename((char *)source_path);
    char *target_path = generate_new_path(dest_path, source_filename);
    
    // Symbolic links or files beginning with a '.' will be skipped.
    if(S_ISLNK(source_info.st_mode) || source_filename[0] == '.') {
    	  fprintf(stdout, "Symbolic links or files beginning with a '.' will be skipped. \n");
        return 1;
    }
        
    if(S_ISREG(source_info.st_mode)) {
        
        FILE *source_file = fopen(source_path, "rb");

        if(source_file == NULL){
            perror("fopen source file");
            return -1;
        }
            
        // Check if the source file exists in the destination.            
        FILE *target_file = fopen(target_path, "rb");
    
        // In this case, the source file doesn't exist in the destination.
        if(target_file == NULL){
    
            // Since by now we know the source file doesn't exist in the destination,
            // we don't need to use fclose because we expect target_file to be NULL.
            target_file = fopen(target_path, "wb");
            
            char c;
            while(fread(&c, sizeof(char), 1, source_file)){
                fwrite(&c, sizeof(char), 1, target_file);                          
            }
                
            // Change the permission to be the same as the source file.
            if(chmod(target_path, source_info.st_mode) != 0){
                perror("chmod");
                return -1;                
            }
                
            if(fclose(target_file) != 0){
                perror("fclose target_file");
                return -1;
            }
        }
            
        // In this case, there is a file exists in the destination
        // that has the same name as the source file.
        else {
            struct stat target_info;
            int target_error = lstat(target_path, &target_info);
    
            if(target_error != 0){
                perror("lstat for target file");
                return -1;
            }
                
            // if the sizes or hash values differ, overwrite the file.
            if((source_info.st_size != target_info.st_size) || strcmp(hash(source_file), hash(target_file)) != 0){
                
                if(fclose(target_file) != 0){
                    perror("fclose target_file");
                    return -1;
                }
                    
                target_file = fopen(target_path, "wb");
                    
                if(target_file == NULL) {
                    perror("fopen target file");
                    return -1;
                }
                    
                char c;
                while(fread(&c, sizeof(char), 1, source_file)){
                    fwrite(&c, sizeof(char), 1, target_file);                          
                }
                
                if(chmod(target_path, source_info.st_mode) != 0){
                    perror("chmod");
                    return -1;                
                }
                
                if(fclose(target_file) != 0){
                    perror("fclose target_file");
                    return -1;
                }
            }
                
        }
            
        if(fclose(source_file) != 0){
            perror("fclose source_file");
            return -1;
        }    
            
    }

    else if(S_ISDIR(source_info.st_mode)) {
        DIR *source_dir = opendir(source_path);

        if(source_dir == NULL) {
            perror("opendir");
            return -1;
        }

        struct dirent *dp = NULL;
            
        // Check if the source directory already exists in destination.            
        DIR *target_dir = opendir(target_path); 

        // In this case, the source directory doesn't exist in destination.
        if(target_dir == NULL){
                
            if(mkdir(target_path, source_info.st_mode) != 0){
                perror("mkdir");
                return -1;
            }
        }    
            
        else{
                
            if(chmod(target_path, source_info.st_mode) != 0){
                perror("chmod");
                return -1;                
            }
                
            if(closedir(target_dir) != 0){
                perror("fclose target_dir");
                return -1;
            }
                            
        }
        while ((dp = readdir(source_dir)) != NULL){
            if(dp->d_name[0] != '.') {
                char *next_source_path = generate_new_path(source_path, dp->d_name);
                struct stat next_source_info;
                int next_source_error = lstat(next_source_path, &next_source_info);
 
                if(next_source_error != 0){
                    perror("lstat for next source");
                    return -1;
                }
                if(S_ISDIR(next_source_info.st_mode)){
                    int r = fork();
                    if(r > 0){
                        int status;
                        if (wait(&status) != -1){
                            if (WIFEXITED(status)){
                                return_value += WEXITSTATUS(status);
                            }
                        }
                        else{
                            // Child exited abnormally
                            return -1;
                        }                    
                    }
                    else if(r == 0){
                        int next_return_value = copy_ftree(next_source_path, target_path);
                        exit(next_return_value);
                    }
                    
                    else if(r < 0){
                        perror("fork");
                        return -1;                    
                    }
                    
                    
                }
                else{
                    copy_ftree(next_source_path, target_path);
                }
                free(next_source_path);
            }
        }
            
        if(closedir(source_dir) != 0){
            perror("fclose source_dir");
            return -1;
        }
    }
    free(target_path);
    return return_value;
}
    