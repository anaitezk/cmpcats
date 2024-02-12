#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "list.h"
#include "compareDirectories.h"

// Function to copy a file into a specified directory
void copyFile(const char *src, const char *dest)
{
    int srcFd, destFd; // File descriptor for source and destination files
    char buffer[BUFSIZ];
    long bytesRead, bytesWritten;

    srcFd = open(src, O_RDONLY); // Open source file only for reading
    if (srcFd == -1)             // Check for correct opening of the file
    {
        perror("open src");
        return;
    }

    destFd = creat(dest, 0644); // Creating the destination file
    if (destFd == -1)           // Checking for correct creation
    {
        perror("open dest");
        close(srcFd);
        return;
    }

    // Copying the source file's contents in the destination file
    while ((bytesRead = read(srcFd, buffer, sizeof(buffer))) > 0) // Reading from the source file
    {
        bytesWritten = write(destFd, buffer, bytesRead); // Writing to the destination file
        if (bytesWritten == -1)                          // Checking for correct writing in destination file
        {
            perror("write");
            close(srcFd);
            close(destFd);
            return;
        }
    }

    if (bytesRead == -1) // Checking for correct reading from source file
    {
        perror("read");
        close(srcFd);
        close(destFd);
        return;
    }

    close(srcFd);  // closing source file
    close(destFd); // closing destination file
    return;
}

int copyDirectory(const char *source, const char *destination, List list)
{
    int count = 0;
    DIR *dir = opendir(source); // Open the source directory
    if (!dir)                   // Checking for correct opening
    {
        perror("opendir");
        return -1;
    }

    // Create destination directory if it doesn't exist
    if (mkdir(destination, 0777) == -1 && errno != EEXIST) // Checking for potentital errors other than the directory existing
    {
        perror("mkdir");
        closedir(dir);
        return -1;
    }

    struct dirent *entry; // Directory entries for source directory

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char sourcePath[2000], destPath[2000];
        sprintf(sourcePath, "%s/%s", source, entry->d_name);
        sprintf(destPath, "%s/%s", destination, entry->d_name); // Update destination path

        struct stat dirstat;
        struct stat deststat;
        if (lstat(sourcePath, &dirstat) == -1)
        {
            perror("stat");
            continue;
        }

        if ((dirstat.st_mode & S_IFMT) == S_IFDIR && searchList(list, sourcePath))
        {
            count++;
            count += copyDirectory(sourcePath, destPath, list); // Recursively copy directories
        }
        else if (dirstat.st_mode & S_IFMT == S_IFREG && searchList(list, sourcePath))
        {
            count++;
            copyFile(sourcePath, destPath);
        }
        else if ((dirstat.st_mode & S_IFMT) == S_IFLNK && searchList(list, sourcePath))
        {
            char buf[1024];
            ssize_t len;
            if ((len = readlink(sourcePath, buf, sizeof(buf) - 1)) != -1)
            {
                buf[len] = '\0';
                printf("buf=%s\n", buf);
                if (symlink(buf, destPath) == -1)
                {
                    perror("Error creating soft link");
                }
            }
            else
            {
                perror("Error reading soft link");
            }
        }
    }

    closedir(dir);
    return count;
}

void mergeDirectories(char *dirA, char *dirB, char *dirC)
{
    List commonPaths = Create();
    List differentPaths = Create();

    findDifferences(dirA, dirB, commonPaths, differentPaths);
    findDifferences(dirB, dirA, commonPaths, differentPaths);

    struct stat dirCstat;
    if (lstat(dirC, &dirCstat) == 0 && (dirCstat.st_mode & S_IFMT) == S_IFDIR) // If the directory exists, delete it.
    {
        // Making sure the user wants the directory to be deleted
        char answer[5];
        printf("Directory %s is about to be deleted, are you sure? Answer yes or no!\n", dirC);
        scanf("%s", answer);
        if (strcmp(answer, "yes") == 0)
        {
            if (rmdir(dirC) != 0)
            {
                perror("rmdir");
                return;
            }
        }
    }

    if (mkdir(dirC, 0777) == -1)
    {
        perror("Failed to create directory");
        return;
    }

    node current = commonPaths->Start;

    for (int i = 0; i < Size(commonPaths) / 2; i++)
    {
        char *dirpath1 = strrchr(dirA, '/');
        char *dirpath2 = strrchr(dirB, '/');
        char *filename = strstr(current->Data, dirpath1);

        if (filename != NULL)
        {
            filename += strlen(dirpath1);
        }

        if (filename == NULL)
        {
            filename = strstr(current->Data, dirpath2);

            if (filename != NULL)
            {
                filename += strlen(dirpath2);
            }
        }

        if (filename != NULL)
        {
            // filename++; // Move past the '/'
            char PathC[1000];
            sprintf(PathC, "%s/%s", dirC, filename);

            // Check if the file already exists in dirC
            struct stat statC;
            struct stat statA;

            if (lstat(current->Data, &statA) == -1)
            {
                perror("stat");
                current = current->Next;
                continue;
            }

            if ((statA.st_mode & S_IFMT) == S_IFDIR)
            {
                int count = copyDirectory(current->Data, PathC, commonPaths);

                for (int i = 0; i < count; i++)
                {
                    if (current->Next != NULL)
                    {
                        current = current->Next;
                    }
                }
            }
            else if (lstat(PathC, &statC) == 0 && (statA.st_mode & S_IFMT) == S_IFREG && (statC.st_mode & S_IFMT) == S_IFREG)
            {
                // If file exists, compare modification dates
                struct stat srcStat;
                if (lstat(current->Data, &srcStat) == 0)
                {
                    // If the src file is newer, copy it to dirC
                    if (difftime(srcStat.st_mtime, statC.st_mtime) > 0)
                    {
                        copyFile(current->Data, PathC);
                    }
                }
            }
            else if ((statA.st_mode & S_IFMT) == S_IFREG)
            {
                copyFile(current->Data, PathC);
            }
            else if ((statA.st_mode & S_IFMT) == S_IFLNK)
            {
                char buf[1024];
                ssize_t len;
                if ((len = readlink(current->Data, buf, sizeof(buf) - 1)) != -1)
                {
                    buf[len] = '\0';
                    printf("buf = %s\n", buf);
                    if (symlink(buf, PathC) == -1)
                    {
                        perror("Error creating soft link");
                    }
                }
                else
                {
                    perror("Error reading soft link");
                }
            }
        }

        current = current->Next;
    }

    current = differentPaths->Start;

    while (current != NULL)
    {
        char *dirpath1 = strrchr(dirA, '/');
        char *dirpath2 = strrchr(dirB, '/');
        char *filename = strstr(current->Data, dirpath1);

        if (filename != NULL)
        {
            filename += strlen(dirpath1);
        }

        if (filename == NULL)
        {
            filename = strstr(current->Data, dirpath2);
            if (filename != NULL)
            {
                filename += strlen(dirpath2);
            }
        }

        if (filename != NULL)
        {
            char PathC[1000];
            sprintf(PathC, "%s%s", dirC, filename);

            // Check if the file already exists in dirC
            struct stat statC;
            struct stat statA;

            if (lstat(current->Data, &statA) == -1)
            {
                perror("stat");
                current = current->Next;
                continue;
            }

            if ((statA.st_mode & S_IFMT) == S_IFDIR)
            {
                int count = copyDirectory(current->Data, PathC, differentPaths);

                for (int i = 0; i < count; i++)
                {
                    current = current->Next;
                }
            }
            else if ((statA.st_mode & S_IFMT) == S_IFREG)
            {
                copyFile(current->Data, PathC);
            }
            else if ((statA.st_mode & S_IFMT) == S_IFLNK)
            {
                char buf[1024];
                ssize_t len;
                if ((len = readlink(current->Data, buf, sizeof(buf) - 1)) != -1)
                {
                    buf[len] = '\0';
                    if (symlink(buf, PathC) == -1)
                    {
                        perror("Error creating soft link");
                    }
                }
                else
                {
                    perror("Error reading soft link");
                }
            }
        }

        current = current->Next;
    }

    deleteList(commonPaths);
    deleteList(differentPaths);

    printf("Merged directory %s has been created\n", dirC);
}