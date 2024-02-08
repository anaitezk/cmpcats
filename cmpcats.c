#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

void compare_directories(const char *dirA, const char *dirB)
{
    DIR *dirA_ptr = opendir(dirA);
    if (!dirA_ptr)
    {
        printf("Failed to open directory %s\n", dirA);
        return;
    }

    DIR *dirB_ptr = opendir(dirB);
    if (!dirB_ptr)
    {
        printf("Failed to open directory %s\n", dirB);
        closedir(dirA_ptr);
        return;
    }

    struct dirent *entryA, *entryB;
    struct stat statA, statB;

    // Traverse dirA
    while ((entryA = readdir(dirA_ptr)) != NULL)
    {
        // Skip current and parent directory
        if (strcmp(entryA->d_name, ".") == 0 || strcmp(entryA->d_name, "..") == 0)
            continue;

        char pathA[1000], pathB[1000];
        sprintf(pathA, "%s/%s", dirA, entryA->d_name);

        // Reset dirB_ptr to the beginning
        rewinddir(dirB_ptr);

        // Flag to indicate if entryA is found in dirB
        int found = 0;

        // Traverse dirB to find entryA
        while ((entryB = readdir(dirB_ptr)) != NULL)
        {
            // Skip current and parent directory
            if (strcmp(entryB->d_name, ".") == 0 || strcmp(entryB->d_name, "..") == 0)
                continue;

            sprintf(pathB, "%s/%s", dirB, entryB->d_name);

            // Get file stats for file in dirA
            if (stat(pathA, &statA) == -1)
            {
                printf("Failed to get stats for file %s\n", pathA);
                continue;
            }

            // Get file stats for file in dirB
            if (stat(pathB, &statB) == -1)
            {
                printf("Failed to get stats for file %s\n", pathB);
                continue;
            }

            // Compare elements
            if ((statA.st_mode & S_IFMT) == S_IFDIR && (statB.st_mode & S_IFMT) == S_IFDIR)
            {

                // If both are directories, recursively compare contents
                if (strcmp(entryA->d_name, ".") != 0 && strcmp(entryA->d_name, "..") != 0 &&
                    strcmp(entryB->d_name, ".") != 0 && strcmp(entryB->d_name, "..") != 0)
                {
                    if (strcmp(entryA->d_name, entryB->d_name) == 0)
                    {
                        found = 1;
                        compare_directories(pathA, pathB);
                        compare_directories(pathB, pathA);
                        break;
                    }
                }
            }
            else if ((statA.st_mode & S_IFMT) == S_IFREG && (statB.st_mode & S_IFMT) == S_IFREG)
            {
                // printf("files %s and %s\n", entryA->d_name, entryB->d_name);
                // If both are regular files, compare their attributes
                if (strcmp(entryA->d_name, entryB->d_name) == 0 && statA.st_size == statB.st_size)
                {
                    found = 1;
                    break;
                }
            }
            else if ((statA.st_mode & S_IFMT) == S_IFLNK && (statB.st_mode & S_IFMT) == S_IFLNK)
            {
                // If both are symbolic links, compare their targets
                // Here you would need to compare the targets, but it's omitted for simplicity.
                if (strcmp(entryA->d_name, entryB->d_name) == 0)
                {
                    found = 1;
                    break;
                }
            }
        }

        if ((statA.st_mode & S_IFMT) == S_IFDIR && found == 0) // In case entryA is a directory and found is 0, print its contents
        {
            DIR *dir = opendir(pathA);
            if (dir == NULL)
            {
                printf("Failed to open directory %s\n", pathA);
                return;
            }

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL)
            {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;

                char path[2000];
                snprintf(path, sizeof(path), "%s/%s", pathA, entry->d_name);
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                {
                    printf("%s\n", path);
                }
            }

            closedir(dir);
        }

        // If entryA is not found in dirB
        if (!found)
        {
            printf("%s\n", pathA);
        }
    }

    // Close directories
    closedir(dirA_ptr);
    closedir(dirB_ptr);
}

int main(int argc, char **argv)
{
    char *dirA = NULL, *dirB = NULL, *dirC = NULL;

    // Checking for the necessary amount of arguments
    if (argc != 4 && argc != 6)
    {
        printf("argc = %d\n", argc);
        printf("Usage: ./cmpcats -d dirA dirB for comparison or ./cmpcats -d dirA dirB -s dirc for merging.\n");
        exit(1);
    }

    // Parse command-line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            dirA = argv[++i];
            dirB = argv[++i];
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            dirC = argv[++i];
        }
    }

    if (argc == 4) // In the case we only need to compare the directories
    {
        printf("In %s:\n", dirA);
        compare_directories(dirA, dirB);

        printf("In %s:\n", dirB);
        compare_directories(dirB, dirA);
    }
    else
    {
        printf("dirA = %s, dirB = %s, dirC = %s\n", dirA, dirB, dirC);
    }

    return 0;
}
