#include "sync.h"

char	*concatenatePaths(const char *path1, const char *path2)
{
    char *result = malloc(MAX_PATH_LENGTH);

    strcpy(result, path1);
    strcat(result, "/");
    strcat(result, path2);
    return (result);
}

void	termination(char *sourcePath, char *replicaPath, DIR *dir, FILE *log, char *message)
{
	if (sourcePath)
		free(sourcePath);
	if (replicaPath)
		free(replicaPath);
	if (dir)
		closedir(dir);
	if (log)
		fclose(log);
	if (message)
		printf("%s\n", message);
}

int	deleteSubfoldersAndFiles(const char *path, FILE *log)
{
	struct dirent *entry;
	struct stat fileStat;
	char replicaPath[FILENAME_MAX];

	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		printf("Error opening directory %s\n", path);
		return (1);
	}
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
			strcpy(replicaPath, path);
			strcat(replicaPath, "/");
			strcat(replicaPath, entry->d_name);
			if (stat(replicaPath, &fileStat) < 0)
			{
				printf("Error getting file stat from %s\n", replicaPath);
				closedir(dir);
				return (1);
			}
			if (S_ISDIR(fileStat.st_mode))
			{
				if (rmdir(replicaPath) == 0)
				{
					printf("Deleted %s\n", replicaPath);
					fprintf(log, "Deleted %s\n", replicaPath);
				}
				else
				{
					if (deleteSubfoldersAndFiles(replicaPath, log) == 0)
					{
						if (rmdir(replicaPath) == 0)
						{
							printf("Deleted %s\n", replicaPath);
							fprintf(log, "Deleted %s\n", replicaPath);
						}
						else
						{
							printf("Error deleting %s\n", replicaPath);
						}
					}
					else
						printf("Error deleting subdirectories and files from %s\n", replicaPath);
				}
			}
			if (S_ISREG(fileStat.st_mode))
			{
				if (remove(replicaPath) == 0)
				{
					printf("Deleted file from %s\n", replicaPath);
					fprintf(log, "Deleted file from %s\n", replicaPath);
				}
				else
					printf("Error deleting %s\n", replicaPath);
			}
		}
	}
	closedir(dir);
	return (0);
}
