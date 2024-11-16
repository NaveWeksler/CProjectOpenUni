#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkedList.h"
#include "../data.h"

void addNodeToList(List *list, Node *newTail)
{
    newTail->next = NULL;

    if (list->tail == NULL)
    {
        list->head = newTail;
        list->tail = newTail;
    }
    else
    {
        list->tail->next = newTail;
        list->tail = newTail;
    }
}

int addToList(List *list, char *key, void *value)
{
    Node *node;
    node = createNode(); /* NOTE: can be NULL */
    if (node == NULL)
    {
        return ERROR_COULD_NOT_ALLOCATE; /* error with malloc. */
    }
    setKey(node, key);
    setValue(node, value);
    addNodeToList(list, node);
    return SUCCESS;
}

/*
 * check if there is a node with some key.
 * list - list to search for key in.
 * key - string to search node in list.
 * return TRUE if found else FALSE
 */
int exists(List *list, char *key)
{
    Node *cur;
    cur = list->head;
    while (cur != NULL)
    {
        if (strcmp(cur->key, key) == 0)
        {
            return TRUE;
        }
        cur = cur->next;
    }
    return FALSE;
}

/*
create a stringNode.
return value should be freed.
*/
Node *createNode()
{
    Node *node;
    node = malloc(sizeof(Node));
    node->key = NULL;
    node->next = NULL;
    node->value = NULL;
    return node;
}

/*
set key of Node.
node - the Node. NOT NULL
key - char pointer (string) to set as key of the Node.
*/
void setKey(Node *node, char *key)
{
    node->key = key;
}

/*
set value of Node.
node - the Node. NOT NULL
key - void pointer to set as value of the Node.
*/
void setValue(Node *node, void *value)
{
    node->value = value;
}