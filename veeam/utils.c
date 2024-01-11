char* concatPaths(const char *path1, const char *path2) {
    char *result = malloc(MAX_PATH_LENGTH);
    strcpy(result, path1);
    strcat(result, "/");
    strcat(result, path2);
    return (result);
}