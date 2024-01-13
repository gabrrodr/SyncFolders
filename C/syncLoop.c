#include "sync.h"

//Synchronize
int	syncFolders(const char *source, const char *replica, const char *logFile)
{
    DIR	*dir;
    struct dirent	*entry;
    struct stat		fileStat;
    char			*sourcePath = NULL;
	char			*replicaPath = NULL;
    FILE *log = NULL;

    if (!(dir = opendir(source)))
	{
        termination(sourcePath, replicaPath, dir, log, "Error opening source directory");
		return (1);	
    }
    if (!(log = fopen(logFile, "a")))
	{
        termination(sourcePath, replicaPath, dir, log, "Error opening log file");
		return (1);
	}
    while ((entry = readdir(dir)) != NULL) 
	{
		//handle leaks
		if (sourcePath)
			free(sourcePath);
		if (replicaPath)
			free(replicaPath);
        //. and .. to handle the current and parent directory cases
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
            sourcePath = concatenatePaths(source, entry->d_name);
            replicaPath = concatenatePaths(replica, entry->d_name);

            if (stat(sourcePath, &fileStat) < 0)
			{
				termination(sourcePath, replicaPath, dir, log, "Error getting file stat");
                return (1);
            }
            if (S_ISDIR(fileStat.st_mode))
			{
				//check if directory exists in replica
				DIR	*rep = opendir(replicaPath);

				if (!rep)
				{
					if (mkdir(replicaPath, 0777) != 0)
					{
						termination(sourcePath, replicaPath, dir, log, NULL);
						printf("Error creating directory %s\n", replicaPath);
						return (1);
					}
					else
					{
						fprintf(log, "\033[38;5;47mCreated directory\033[0m %s\n", replicaPath);
						printf("\033[38;5;47mCreated directory\033[0m %s\n", replicaPath);
					}
				}
				closedir(rep);
				syncFolders(sourcePath, replicaPath, logFile);
				// Recursively synchronize subdirectories
            }
			if (S_ISREG(fileStat.st_mode))
			{
				// check if the entry exists in both source and replica
				if (access(sourcePath, F_OK) != -1 && access(replicaPath, F_OK) != -1)
				{
					FILE	*src;
					FILE	*rep;
            		int 	ch1;
					int		ch2;
            		int		isDifferent = 0;

            		src = fopen(sourcePath, "rb");
            		rep = fopen(replicaPath, "rb");

					if (src == NULL || rep == NULL) 
					{
						if (src == NULL)
							printf ("Error: unable to open %s\n", sourcePath);
						else
							printf ("Error: unable to open %s\n", replicaPath);
                		if (src != NULL)
        					fclose(src);
    					if (rep != NULL)
        					fclose(rep);
                		continue;
            		}
					while ((ch1 = fgetc(src)) != EOF && (ch2 = fgetc(rep)) != EOF)
					{
                		if (ch1 != ch2)
						{
                    		isDifferent = 1;
                    		break;
                		}
           			}
					fclose(src);
					fclose(rep);
					if (isDifferent)
					{
                		// Remove the file from replicaPath
               			if (remove(replicaPath) == 0)
						{
                    		fprintf(log, "\033[38;5;196mDeleted \033[0m%s from %s\n", entry->d_name, replicaPath);
                    		printf("\033[38;5;196mDeleted \033[0m%s from %s\n", entry->d_name, replicaPath);
                		}
						else
						{
                    		fprintf(log, "Error deleting %s from %s\n", entry->d_name, replicaPath);
                    		printf("Error deleting %s from %s\n", entry->d_name, replicaPath);
							termination(sourcePath, replicaPath, NULL, NULL, NULL);
                    		continue;
                		}
                		// Copy the file from source to replica
                		copyFile(sourcePath, replicaPath);
                		fprintf(log, "\033[38;5;226mCopied \033[0m%s to %s\n", sourcePath, replicaPath);
                		printf("\033[38;5;226mCopied \033[0m%s to %s\n", sourcePath, replicaPath);
            		}
					else
					{
						continue;
					}
				}
				// If the entry exists in source but not in replica
                else if (access(replicaPath, F_OK) == -1 && access(sourcePath, F_OK) != -1)
				{
                    copyFile(sourcePath, replicaPath);
                    fprintf(log, "\033[38;5;226mCopied \033[0m%s to %s\n", sourcePath, replicaPath);
                    printf("\033[38;5;226mCopied \033[0m%s to %s\n", sourcePath, replicaPath);
                }
            } 
		}
	}
	closedir(dir);
	if (checkReplica(source, replica, log))
	{
		termination(sourcePath, replicaPath, NULL, NULL, NULL);
		return (1);
	}
	termination(sourcePath, replicaPath, NULL, NULL, NULL);
    fclose(log);
	return (0);
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        printf("ERROR: correct -> ./programName source replica interval logFile\n");
        return (1);
    }
    
    const char *source = argv[1];
    const char *replica = argv[2];
    int interval = atoi(argv[3]);
    const char *logFile = argv[4];

    while (1)
    {
        if (syncFolders(source, replica, logFile) == 1)
        {
            printf("Error with synchronization\n");
            return (1);
        }
		printf("SYNCHRONIZATION DONE\n");
        sleep(interval);
    }
    return (0);
}