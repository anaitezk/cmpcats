#ifndef LIST
#define LIST

typedef struct Node
{
    char *Data;
    struct Node *Next;
} Node;

typedef struct LinkedList
{
    int Count;
    struct Node *Start;
} LinkedList;

typedef struct Node *node;
typedef struct LinkedList *List;

List Create();
int Size(List list);
int IsEmpty(List list);
void push(List list, char *item);
void PrintList(List list);
void deleteList(List list);
int searchList(List list, char* item);

#endif