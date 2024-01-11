//file synchronization in C

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
void	syncFolders(const char *source, const char *replica, const char *logFile)
{
    DIR	*dir;
    struct dirent	*entry;
    struct stat	fileStat;
    char	*sourcePath, *replicaPath;
    FILE *log;

    if (!(dir = opendir(source))) {
        printf("Error opening source directory\n");
        return;
    }

    if (!(log = fopen(logFile, "a"))) {
        printf("Error opening log file\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            sourcePath = concatPaths(source, entry->d_name);
            replicaPath = concatPaths(replica, entry->d_name);

            if (stat(sourcePath, &fileStat) < 0) {
                printf("Error getting file stat\n");
                return;
            }

            if (S_ISDIR(fileStat.st_mode)) {
                syncFolders(sourcePath, replicaPath, logFile); // Recursively synchronize subdirectories
            } else {
                if (access(replicaPath, F_OK) == -1) {
                    copyFile(sourcePath, replicaPath);
                    fprintf(log, "Copied %s to %s\n", sourcePath, replicaPath);
                    printf("Copied %s to %s\n", sourcePath, replicaPath);
                }
            }

            free(sourcePath);
            free(replicaPath);

            // If the entry exists in both source and replica
        if (access(sourcePath, F_OK) != -1 && access(replicaPath, F_OK) != -1) {
            FILE *src, *rep;
            int ch1, ch2;
            int isDifferent = 0;

            src = fopen(sourcePath, "rb");
            rep = fopen(replicaPath, "rb");

            if (src == NULL || rep == NULL) {
                // Error handling: unable to open files
                fclose(src);
                fclose(rep);
                continue;
            }

            while ((ch1 = fgetc(src)) != EOF && (ch2 = fgetc(rep)) != EOF) {
                if (ch1 != ch2) {
                    isDifferent = 1;
                    break;
                }
            }

            fclose(src);
            fclose(rep);

            if (isDifferent) {
                // Remove the file from replicaPath
                if (remove(replicaPath) == 0) {
                    fprintf(log, "Deleted %s from %s\n", entry->d_name, replica);
                    printf("Deleted %s from %s\n", entry->d_name, replica);
                } else {
                    fprintf(log, "Error deleting %s from %s\n", entry->d_name, replica);
                    printf("Error deleting %s from %s\n", entry->d_name, replica);
                    continue; // Skip copying if deletion fails
                }

                // Copy the file from source to replica
                copyFile(sourcePath, replicaPath);
                fprintf(log, "Copied %s to %s\n", sourcePath, replicaPath);
                printf("Copied %s to %s\n", sourcePath, replicaPath);
            }
             if (access(sourcePath, F_OK) == -1 && access(replicaPath, F_OK) != -1) {
            if (S_ISREG(fileStat.st_mode)) {
                // Remove the file from replicaPath
                if (remove(replicaPath) == 0) {
                    fprintf(log, "Deleted %s from %s\n", entry->d_name, replica);
                    printf("Deleted %s from %s\n", entry->d_name, replica);
                } else {
                    fprintf(log, "Error deleting %s from %s\n", entry->d_name, replica);
                    printf("Error deleting %s from %s\n", entry->d_name, replica);
                }
            } else if (S_ISDIR(fileStat.st_mode)) {
                deleteSubfoldersAndFiles(const char *path)
                {
                    struct dirent *entry;
    struct stat fileStat;
    char fullPath[FILENAME_MAX];

    if (!(dir = opendir(path))) {
        fprintf(stderr, "Error opening directory: %s\n", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

            if (stat(fullPath, &fileStat) < 0) {
                fprintf(stderr, "Error getting file stat: %s\n", fullPath);
                continue;
            }

            if (S_ISDIR(fileStat.st_mode)) {
                deleteSubfoldersAndFiles(fullPath); // Recursive call for subdirectories
            } else {
                if (remove(fullPath) == 0) {
                    printf("Deleted file: %s\n", fullPath);
                } else {
                    fprintf(stderr, "Error deleting file: %s\n", fullPath);
                }
            }
        }
    }

    closedir(dir);

    // Delete the directory itself after its content is cleared
    if (rmdir(path) == 0) {
        printf("Deleted directory: %s\n", path);
    } else {
        fprintf(stderr, "Error deleting directory: %s\n", path);
    }
}

int main() {
    const char *replicaPath = "/path/to/replica"; // Replace with your replica path
    deleteSubfoldersAndFiles(replicaPath);
    return 0;
}
                }
                // Remove the directory recursively from replicaPath
                // (You'll need a recursive delete function here)
                // Example: Implement a recursive function similar to syncFolders to delete subdirectories and files in replicaPath

        }
    }

    closedir(dir);
    fclose(log);
}

int syncFolders(const char *source, const char *replica, const char *logFile)
{
    DIR *directories;

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
        sleep(interval);
    }
    return (0);
}