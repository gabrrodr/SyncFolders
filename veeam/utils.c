#include "sync.h"
//Copy file from source to replica (char by char)
void	copyFile(const char *source, const char *replica)
{
    FILE	*src, *rep;
    char	ch;

    src = fopen(source, "rb");
    rep = fopen(replica, "wb");

    while ((ch = fgetc(src)) != EOF)
        fputc(ch, rep);
    fclose(src);
    fclose(rep);
}

char	*concatenatePaths(const char *path1, const char *path2)
{
    char *result = malloc(strlen(path1) + strlen(path2) + 2);

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
					printf("\033[38;5;196mDeleted \033[0m%s\n", replicaPath);
					fprintf(log, "\033[38;5;196mDeleted \033[0m%s\n", replicaPath);
				}
				else
				{
					if (deleteSubfoldersAndFiles(replicaPath, log) == 0)
					{
						if (rmdir(replicaPath) == 0)
						{
							printf("\033[38;5;196mDeleted \033[0m%s\n", replicaPath);
							fprintf(log, "\033[38;5;196mDeleted \033[0m%s\n", replicaPath);
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
					printf("\033[38;5;196mDeleted \033[0mfile from %s\n", replicaPath);
					fprintf(log, "Deleted \033[0mfile from %s\n", replicaPath);
				}
				else
					printf("Error deleting %s\n", replicaPath);
			}
		}
	}
	closedir(dir);
	return (0);
}

int	checkReplica(const char *source, const char *replica, FILE *log)
{
	DIR	*dir;
    struct dirent	*entry;
    struct stat		fileStat;
    char			*sourcePath = NULL;
	char			*replicaPath = NULL;

	dir = opendir(replica);
	if (dir == NULL)
	{
		termination(sourcePath, replicaPath, dir, log, "Error opening replica directory");
		return (1);
	}
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
			if (sourcePath)
				free(sourcePath);
			if (replicaPath)
				free(replicaPath);

			sourcePath = concatenatePaths(source, entry->d_name);
			replicaPath = concatenatePaths(replica, entry->d_name);

			if (stat(replicaPath, &fileStat) < 0)
			{
				termination(sourcePath, replicaPath, dir, log, "Error getting file stat from replica");
				return (1);
			}
			if (S_ISDIR(fileStat.st_mode) && access(sourcePath, F_OK) == -1)
			// If the entry exists in replica but not in source
			{
				if (deleteSubfoldersAndFiles(replicaPath, log))
				{
					termination(sourcePath, replicaPath, dir, log, "Error deleting subfolders and files from replica");
					continue;
				}
				if (rmdir(replicaPath) == 0)
				{
					printf("\033[38;5;196mDeleted \033[0m%s\n", replicaPath);
					fprintf(log, "\033[38;5;196mDeleted \033[0m%s\n", replicaPath);
				}
			}
			if (S_ISREG(fileStat.st_mode))
			{
				if (access(sourcePath, F_OK) == -1)
				{
					if (remove(replicaPath) == 0)
					{
						fprintf(log, "\033[38;5;196mDeleted \033[0m%s from %s\n", entry->d_name, replica);
						printf("\033[38;5;196mDeleted \033[0m%s from %s\n", entry->d_name, replica);
					}
					else
					{
						fprintf(log, "Error deleting %s from %s\n", entry->d_name, replica);
						printf("Error deleting %s from %s\n", entry->d_name, replica);
					}
				}
			}
		}
	}
	termination(sourcePath, replicaPath, NULL, NULL, NULL);
    closedir(dir);
	return (0);
}
