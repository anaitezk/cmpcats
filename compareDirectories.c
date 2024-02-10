#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int compareFileContents(const char *fileA, const char *fileB) // Function to compare the contents of 2 files.
                                                              // Returns 1 only if the files have the same content
{
    int fdA, fdB;
    long sizeA, sizeB;
    long bytesReadA, bytesReadB;

    // Open files
    fdA = open(fileA, O_RDONLY);
    if (fdA == -1) // Check for correct opening
    {
        perror("Failed to open file A");
        return -1;
    }
    fdB = open(fileB, O_RDONLY);
    if (fdB == -1) // Check for correct opening
    {
        perror("Failed to open file B");
        close(fdA);
        return -1;
    }

    // Get size of file A
    sizeA = lseek(fdA, 0, SEEK_END);
    if (sizeA == -1) // Check for correct operation of lseek
    {
        perror("Failed to get file size of file A");
        close(fdA);
        close(fdB);
        return -1;
    }

    // Get size of file B
    sizeB = lseek(fdB, 0, SEEK_END);
    if (sizeB == -1) // Check for correct operation of lseek
    {
        perror("Failed to get file size of file B");
        close(fdA);
        close(fdB);
        return -1;
    }

    // Compare file sizes
    if (sizeA == sizeB)
    {
        // Compare file contents
        char *bufferA = malloc(sizeA); // Allocate memory for buffer
        if (bufferA == NULL)           // check for correct allocation
        {
            perror("Failed to allocate memory for buffer A");
            free(bufferA);
            close(fdA);
            close(fdB);
            return -1;
        }
        char *bufferB = malloc(sizeB); // Allocate memory for buffer
        if (bufferB == NULL)           // check for correct alloation
        {
            perror("Failed to allocate memory for buffer B");
            free(bufferA);
            free(bufferB);
            close(fdA);
            close(fdB);
            return -1;
        }

        lseek(fdA, 0, SEEK_SET); // Reset file pointers to beginning
        lseek(fdB, 0, SEEK_SET);

        bytesReadA = read(fdA, bufferA, sizeA); // Read all the contents of fileA
        bytesReadB = read(fdB, bufferB, sizeB); // Read all the contents of fileB

        if (memcmp(bufferA, bufferB, sizeA) != 0) // Check if contents are different
        {
            // printf("The file contents are different\n");
            free(bufferA);
            free(bufferB);
            close(fdA);
            close(fdB);
            return 0; // Contents differ
        }

        free(bufferA); // Deallocate buffer memory
        free(bufferB); // Deallocate buffer memory
        close(fdA);    // Close fileA
        close(fdB);    // Close fileB
    }

    return 1; // Contents are the same
}

int findDifferences(const char *dirA, const char *dirB) // Function which recursively checks if the contents of dirA are also found in dirB
                                                        // It returns 0 only in case the firectories are identical, non 0 value otherwise.
{
    int count = 0;
    DIR *dirA_ptr = opendir(dirA); // Open dirA
    if (!dirA_ptr)                 // Check for correct opening of directory
    {
        printf("Failed to open directory %s\n", dirA);
        return -1;
    }

    DIR *dirB_ptr = opendir(dirB); // Open dirB
    if (!dirB_ptr)                 // Check for correct opening of directory
    {
        printf("Failed to open directory %s\n", dirB);
        closedir(dirA_ptr);
        return -1;
    }

    struct dirent *entryA, *entryB; // Directory entries for dirA,dirB
    struct stat statA, statB;       // structs for the stats of dirA and dirB

    // Traverse dirA as long as there are contents
    while ((entryA = readdir(dirA_ptr)) != NULL)
    {
        // Skip current and parent directory
        if (strcmp(entryA->d_name, ".") == 0 || strcmp(entryA->d_name, "..") == 0)
            continue;

        char pathA[1000], pathB[1000];
        sprintf(pathA, "%s/%s", dirA, entryA->d_name); // get the complete path of entryA

        // Reset dirB_ptr to the beginning
        rewinddir(dirB_ptr);

        // Flag to indicate if entryA is found in dirB
        int found = 0;

        // Traverse dirB to find entryA as long as there are contents
        while ((entryB = readdir(dirB_ptr)) != NULL)
        {
            // Skip current and parent directory
            if (strcmp(entryB->d_name, ".") == 0 || strcmp(entryB->d_name, "..") == 0)
                continue;

            sprintf(pathB, "%s/%s", dirB, entryB->d_name); // get full path of entryB

            // Get file stats for element in dirA
            if (stat(pathA, &statA) == -1)
            {
                printf("Failed to get stats for file %s\n", pathA);
                continue;
            }

            // Get file stats for element in dirB
            if (stat(pathB, &statB) == -1)
            {
                printf("Failed to get stats for file %s\n", pathB);
                continue;
            }

            // Compare elements

            // If both are directories
            if ((statA.st_mode & S_IFMT) == S_IFDIR && (statB.st_mode & S_IFMT) == S_IFDIR)
            {
                // Compare their names
                if (strcmp(entryA->d_name, ".") != 0 && strcmp(entryA->d_name, "..") != 0 &&
                    strcmp(entryB->d_name, ".") != 0 && strcmp(entryB->d_name, "..") != 0)
                {
                    // If they have the same names recursively compare their contents
                    if (strcmp(entryA->d_name, entryB->d_name) == 0)
                    {
                        found = 1;
                        findDifferences(pathA, pathB);
                        break;
                    }
                }
            }
            // If both are files
            else if ((statA.st_mode & S_IFMT) == S_IFREG && (statB.st_mode & S_IFMT) == S_IFREG)
            {
                // Compare their names, sizes and contents
                if (strcmp(entryA->d_name, entryB->d_name) == 0 && statA.st_size == statB.st_size && compareFileContents(pathA, pathB) == 1)
                {
                    found = 1;
                    break;
                }
            }
            // If both are symbolic links
            else if ((statA.st_mode & S_IFMT) == S_IFLNK && (statB.st_mode & S_IFMT) == S_IFLNK)
            {
                // Compare their names
                if (strcmp(entryA->d_name, entryB->d_name) == 0 && statA.st_ino == statB.st_ino)
                {
                    found = 1;
                    break;
                }
            }
        }

        if ((statA.st_mode & S_IFMT) == S_IFDIR && found == 0) /* In case entryA is a directory and found is 0
        (which means that the directories are not the same, so their contents should be printed to indicate the difference),
        print its contents */
        {
            DIR *dir = opendir(pathA); // Open directory
            if (dir == NULL)           // Check for correct opening
            {
                printf("Failed to open directory %s\n", pathA);
                return -1;
            }

            struct dirent *entry; // Directory entry

            // Traverse the directory as long as there are contents
            while ((entry = readdir(dir)) != NULL)
            {
                // Skip current and parent directory
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue;

                char path[2000];
                sprintf(path, "%s/%s", pathA, entry->d_name); // get the full path of the entry

                printf("%s\n", path); // print the full path

                struct stat substatA; // struct for the stats of path

                if (stat(path, &substatA) == 0 && (substatA.st_mode & S_IFMT) == S_IFDIR) // Check if path is a directory
                {
                    findDifferences(path, dirB); // Recursively compare subdirectory contents in order to print them as we already know they are different
                }
            }
            closedir(dir); // close the directory
        }

        // If entryA is not found in dirB
        if (!found)
        {
            count++;
            printf("%s\n", pathA); // print the full path to entryA
        }
    }

    // Close directories
    closedir(dirA_ptr);
    closedir(dirB_ptr);

    // return the count which indicates if there are differences
    return count;
}

void compareDirectories(char *dirA, char *dirB) // Function to call the findDifferencies function
{
    printf("In %s:\n", dirA); // First checking if the contents of dirA are found in dirB
    int diffs1 = findDifferences(dirA, dirB);

    printf("In %s:\n", dirB); // Then checking if the contents of dirB are fount in dirA
    int diffs2 = findDifferences(dirB, dirA);

    if (!diffs1 && !diffs2) // If there are no differencies
    {
        printf("No differences between %s and %s\n", dirA, dirB); // print a message
    }
}