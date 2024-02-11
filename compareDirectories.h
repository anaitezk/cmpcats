#ifndef COMPARE_DIRS
#define COMPARE_DIRS
#include "list.h"

int findDifferences(const char *dirA, const char *dirB, List commonPaths, List differentPaths);
void compareDirectories(char *dirA, char *dirB);

#endif