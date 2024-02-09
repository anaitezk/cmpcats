#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compareDirectories.h"


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
        compareDirectories(dirA,dirB);
    }
    else
    {
        printf("dirA = %s, dirB = %s, dirC = %s\n", dirA, dirB, dirC);
    }

    return 0;
}
