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
char*   concatenatePaths(const char *path1, const char *path2);

//copy the file
void    copyFile(const char *source, const char *replica);

//What happens when there is a problem and the program should stop the sync
void	termination(char *sourcePath, char *replicaPath, DIR *dir, FILE *log, char *message);

//where I delete things from the replica
int		deleteSubfoldersAndFiles(const char *path, FILE *log);

//When Im checking the replica to see if something should
//be deleted cause it doesnt exist in the source
int	checkReplica(const char *source, const char *replica, FILE *log);

#endif