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
        //. and .. to handle the current and parent directory cases
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
			//handle leaks
			if (sourcePath)
				free(sourcePath);
			if (replicaPath)
				free(replicaPath);

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
						fprintf(log, "Created directory %s\n", replicaPath);
						printf("Created directory %s\n", replicaPath);
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
                    		fprintf(log, "Deleted %s from %s\n", entry->d_name, replica);
                    		printf("Deleted %s from %s\n", entry->d_name, replica);
                		}
						else
						{
                    		fprintf(log, "Error deleting %s from %s\n", entry->d_name, replica);
                    		printf("Error deleting %s from %s\n", entry->d_name, replica);
                    		continue; // Skip copying if deletion fails
                		}
                		// Copy the file from source to replica
                		copyFile(sourcePath, replicaPath);
                		fprintf(log, "Copied %s to %s\n", sourcePath, replicaPath);
                		printf("Copied %s to %s\n", sourcePath, replicaPath);
            		}
					else
						continue;
				}
				// If the entry exists in source but not in replica
                else if (access(replicaPath, F_OK) == -1 && access(sourcePath, F_OK) != -1)
				{
                    copyFile(sourcePath, replicaPath);
                    fprintf(log, "Copied %s to %s\n", sourcePath, replicaPath);
                    printf("Copied %s to %s\n", sourcePath, replicaPath);
                }
            } 
		}
	}
	closedir(dir);
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
					printf("Deleted %s\n", replicaPath);
					fprintf(log, "Deleted %s\n", replicaPath);
				}
			}
			if (S_ISREG(fileStat.st_mode))
			{
				if (access(sourcePath, F_OK) == -1)
				{
					if (remove(replicaPath) == 0)
					{
						fprintf(log, "Deleted %s from %s\n", entry->d_name, replica);
						printf("Deleted %s from %s\n", entry->d_name, replica);
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
    closedir(dir);
    fclose(log);
	return (0);
}

/*
Parse arguments needed
Error message (if number of args wrong)
Need infinite loop because periodic synchronization (interval in seconds)
Const char because i dont want the functions to make alterations to these
*/

/*
open source directory and open LogFile
    in logFile i write alterations 
        file creation/copying/removal
Iterate through files in Source dir (DIR)
    -is it file or a directory?
    (stat() to get info and then see what it is)
    directory?
        -recursive (subdirectories)
    file?
        -copy to replica
                check if the entry is in both but theyre different (byte by byte)
                    fopen and read both, if one is null continue
                    fgetc(src) and fget(path)
                    if EOF == -1 top
                    remove the one in replica and copy the one from source
                

            -open source and replica file
            -read from source and write to replica
            -close source and replica file
    fopen (logFile) in append mode
close source directory and log file
Output must have the logFile operations

finds the files, sees the path of said file in the source, puts it in the same path but instead
of source its replica
*/

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