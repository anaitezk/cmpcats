#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

List Create()
{
    List list = malloc(sizeof(*list)); // Allocating enough memory for a list
    list->Count = 0;                   // Initialising every data member of the struct
    list->Start = NULL;
    return list; // Returning list
}

int Size(List list)
{
    return list->Count;
}

int IsEmpty(List list)
{
    return (list->Start == NULL); // Returning 1 if list is empty,0 otherwise
}

void push(List list, char *item)
{
    node newNode = malloc(sizeof(*newNode)); // Allocating enough memory for a node
    newNode->Data = (char *)malloc(strlen(item) + 1);
    strcpy(newNode->Data, item); // Inserting new node's data

    newNode->Next = list->Start;
    list->Start = newNode;
    list->Count++; // Increase list's size by one

    return;
}

void PrintList(List list)
{
    node N;

    N = list->Start;
    while (N != NULL)
    {
        printf("%s", N->Data);
        N = N->Next;
        printf("\n ");
    }
}

void deleteList(List list)
{
    node N = list->Start;

    while (N != NULL)
    {

        node temp = N;
        N = N->Next;
        free(temp->Data);
        free(temp);
    }
    free(list);
}

int searchList(List list, char *item)
{
    node N = list->Start;

    while (N != NULL)
    {
        if (strcmp(N->Data, item) == 0)
        {
            return 1;
        }

        N = N->Next;
    }

    return 0;
}
