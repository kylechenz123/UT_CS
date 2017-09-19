#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include "ftree.h"
#include "hash.h"

/* Returns a new path by combining the current path
*  and the given filename with a slash as the separator in the middle.
*/
char *generate_new_path(const char * current_path, char *fname){
    char *separator = "/";
    int space_needed = strlen(separator) + strlen(current_path)
                       + strlen(fname) + 1;

    char *new_path = malloc(space_needed);

    if(new_path == NULL){
      fprintf(stderr, "Unable to allocate memory for new path.");
      exit(1);
    }

    int i;
    for (i = 0; i < space_needed + 1; i++){
      new_path[i] = '\0';
    }

    strncpy(new_path, (char *) current_path, strlen((char *) current_path) );
    strncat(new_path, separator, strlen(separator));
    strncat(new_path, fname, strlen(fname));

    return new_path;
}

/*
 * Returns the FTree rooted at the path fname.
 */
struct TreeNode *generate_ftree(const char *fname) {

    struct stat node_info;
    int error = lstat(fname, &node_info);

    if(error != 0){
      perror("lstat");
      exit(1);
    }

    struct TreeNode *node = malloc(sizeof(struct TreeNode));

    if(node == NULL){
      fprintf(stderr, "Unable to allocate memory for current node.");
      exit(1);
    }
    
    // Use mathod basename() to extract the filename from the current path.
    node->fname = basename((char *) fname);
    node->permissions = (node_info.st_mode & 0777);
    node->contents = NULL;
    node->hash = NULL;
    node->next = NULL;

    if(S_ISDIR(node_info.st_mode)){

      DIR *dirp = opendir(fname);

      if(dirp == NULL) {
        perror("opendir");
        exit(1);
      }

      struct dirent *dp = readdir(dirp);

      struct TreeNode *current_node = NULL;

      // Walk through all the files/directories under the current directory
      // to construct a linked list for node->contents.
      while (dp != NULL){
        if(dp->d_name[0] != '.') {
        	
          // Use the helper function generate_new_path() to create a new path
          // for the next generate_ftree() call. 
          char * new_fname = generate_new_path(fname, dp->d_name);

          if(node->contents == NULL){
            current_node = generate_ftree(new_fname);
            node->contents = current_node;
          }

          else{
            current_node->next = generate_ftree(new_fname);
            current_node = current_node->next;
          }
          

        }

        dp = readdir(dirp);
      }



      int error = closedir(dirp);
      if (error != 0){
        perror("closedir");
        exit(1);
      }

    }

    else if(S_ISREG(node_info.st_mode) || S_ISLNK(node_info.st_mode)){

      FILE *source_file = fopen(fname, "r");

      if(source_file == NULL){
        perror("fopen");
        exit(1);
      }

      node->hash = hash(source_file);

      int error = fclose(source_file);
      if(error != 0){
        perror("fclose");
        exit(1);
      }
    }
    return node;
}



/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 */
void print_ftree(struct TreeNode *root) {

  static int depth = 0;
  printf("%*s", depth * 2, "");

  // This case is only true when root is not a directory.
  if(root->contents == NULL && root->hash != NULL){
    printf("%s (%o)\n", root->fname, root->permissions);
  }
  else{
    printf("===== %s (%o) =====\n", root->fname, root->permissions);
    struct TreeNode *current_node = root->contents;
    while (current_node != NULL){
        depth ++;
        print_ftree(current_node);
        depth --;
        current_node = current_node->next;
    }

  }

}
