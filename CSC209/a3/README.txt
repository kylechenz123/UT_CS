    To detect hard linked files, the program will need to extract
their inodes and compare them. However, since a child process is created 
whenever a subdirectory is found, if there is a file in a subdirectory
that is hard linked with another file in the parent directory, it will
be difficult to compare the two inodes because the parent process will have
to compare the inode with every regular files found in its child / grandchild
processes. This process can be costly and complicated, and that is why using
this design makes it difficult to detect hard links.
