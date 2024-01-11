#ifndef SYNC_H
# define SYNC_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <dirent.h>
# include <sys/stat.h>
# include <sys/types.h>

#define MAX_PATH_LENGTH 1024

//concatenate paths
char*   concatPaths(const char *path1, const char *path2);

//copy the file
void    copyFile(const char *source, const char *replica);



#endif