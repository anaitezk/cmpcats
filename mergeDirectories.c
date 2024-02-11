#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "list.h"
#include "compareDirectories.h"

int copyFile(const char *src, const char *dest)
{
    int srcFd, destFd;
    char buffer[BUFSIZ];
    ssize_t bytesRead, bytesWritten;

    srcFd = open(src, O_RDONLY);
    if (srcFd == -1)
    {
        perror("open src");
        return -1;
    }

    destFd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (destFd == -1)
    {
        perror("open dest");
        close(srcFd);
        return -1;
    }

    while ((bytesRead = read(srcFd, buffer, sizeof(buffer))) > 0)
    {
        bytesWritten = write(destFd, buffer, bytesRead);
        if (bytesWritten == -1)
        {
            perror("write");
            close(srcFd);
            close(destFd);
            return -1;
        }
    }

    if (bytesRead == -1)
    {
        perror("read");
        close(srcFd);
        close(destFd);
        return -1;
    }

    close(srcFd);
    close(destFd);
    return 0;
}

int copyDirectory(const char *source, const char *destination)
{
    int count = 0;
    DIR *dir = opendir(source); // Open the source directory
    if (!dir)
    {
        perror("opendir");
        return -1;
    }

    // Create destination directory if it doesn't exist
    if (mkdir(destination, 0777) == -1 && errno != EEXIST)
    {
        perror("mkdir");
        closedir(dir);
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char sourcePath[2000], destPath[2000];
        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", source, entry->d_name);
        snprintf(destPath, sizeof(destPath), "%s/%s", destination, entry->d_name); // Update destination path

        struct stat dirstat;
        struct stat deststat;
        if (stat(sourcePath, &dirstat) == -1)
        {
            perror("stat");
            continue;
        }

        if ((dirstat.st_mode & S_IFMT) == S_IFDIR)
        {
            count += copyDirectory(sourcePath, destPath); // Recursively copy directories
        }
        else if (stat(destPath, &deststat) == 0 && (dirstat.st_mode & S_IFMT) == S_IFREG)
        {
            // If file exists, compare modification dates
            struct stat srcStat;
            if (stat(sourcePath, &srcStat) == 0)
            {
                // If the src file is newer, copy it to dirC
                if (difftime(srcStat.st_mtime, dirstat.st_mtime) > 0)
                {
                    count++;
                    copyFile(sourcePath, destPath);
                }
            }
        }
        else
        {
            count++;
            copyFile(sourcePath, destPath);
        }
    }

    closedir(dir);
    return count;
}

void mergeDirectories(char *dirA, char *dirB, char *dirC)
{
    List commonPaths = Create();
    List differentPaths = Create();
    List copiedDirs = Create();

    findDifferences(dirA, dirB, commonPaths, differentPaths);
    findDifferences(dirB, dirA, commonPaths, differentPaths);

    if (mkdir(dirC, 0777) == -1)
    {
        printf("Failed to create directory %s\n", dirC);
        return;
    }

    node current = commonPaths->Start;

    for (int i = 0; i < Size(commonPaths) / 2; i++)
    {
        char *filename = strrchr(current->Data, '/');

        if (filename != NULL)
        {
            filename++; // Move past the '/'
            char PathC[1000];
            snprintf(PathC, sizeof(PathC), "%s/%s", dirC, filename);

            // Check if the file already exists in dirC
            struct stat statC;
            struct stat statA;

            if (stat(current->Data, &statA) == -1)
            {
                perror("stat");
                current = current->Next;
                continue;
            }

            if ((statA.st_mode & S_IFMT) == S_IFDIR)
            {
                int count = copyDirectory(current->Data, PathC);

                for (int i = 0; i < count; i++)
                {
                    current = current->Next;
                }
            }
            else if (stat(PathC, &statC) == 0 && (statA.st_mode & S_IFMT) == S_IFREG && (statC.st_mode & S_IFMT) == S_IFREG)
            {
                // If file exists, compare modification dates
                printf("PathC = %s\n", PathC);
                struct stat srcStat;
                if (stat(current->Data, &srcStat) == 0)
                {
                    // If the src file is newer, copy it to dirC
                    if (difftime(srcStat.st_mtime, statC.st_mtime) > 0)
                    {
                        copyFile(current->Data, PathC);
                    }
                }
            }
            else if((statA.st_mode & S_IFMT) == S_IFREG)
            {
                copyFile(current->Data, PathC);
            }
        }

        current = current->Next;
    }

    current = differentPaths->Start;

    while (current != NULL)
    {
        char *filename = strrchr(current->Data, '/');

        if (filename != NULL)
        {
            filename++; // Move past the '/'
            char PathC[1000];
            snprintf(PathC, sizeof(PathC), "%s/%s", dirC, filename);

            // Check if the file already exists in dirC
            struct stat statC;
            struct stat statA;

            if (stat(current->Data, &statA) == -1)
            {
                perror("stat");
                current = current->Next;
                continue;
            }

            if ((statA.st_mode & S_IFMT) == S_IFDIR)
            {
                int count = copyDirectory(current->Data, PathC);

                for (int i = 0; i < count; i++)
                {
                    current = current->Next;
                }
            }
            else if (stat(PathC, &statC) == 0 && (statA.st_mode & S_IFMT) == S_IFREG && (statC.st_mode & S_IFMT) == S_IFREG)
            {
                // If file exists, compare modification dates
                printf("PathC = %s\n", PathC);
                struct stat srcStat;
                if (stat(current->Data, &srcStat) == 0)
                {
                    // If the src file is newer, copy it to dirC
                    if (difftime(srcStat.st_mtime, statC.st_mtime) > 0)
                    {
                        copyFile(current->Data, PathC);
                    }
                }
            }
            else if((statA.st_mode & S_IFMT) == S_IFREG)
            {
                copyFile(current->Data, PathC);
            }
        }

        current = current->Next;
    }

    deleteList(commonPaths);
    deleteList(differentPaths);
}