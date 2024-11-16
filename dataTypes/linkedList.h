#ifndef LinkedList_H
#define LinkedList_H

typedef struct node
{
    char *key;
    void *value;
    struct node *next;
} Node;

typedef struct
{
    Node *head;
    Node *tail;
} List;

void addNodeToList(List *, Node *);
int addToList(List *, char *, void *);
int exists(List *, char *);
Node *createNode();
void setKey(Node *, char *);
void setValue(Node *, void *);

#endif