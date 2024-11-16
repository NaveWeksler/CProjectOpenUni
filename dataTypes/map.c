/*
    O(n) get, add time complexity.
    can be much faster.
*/
#include "linkedList.h"
#include <stdio.h>
#include <string.h>
#include "map.h"
#include "linkedList.h"
#include "../data.h"

/*
 * get value by key.
 * @param map stringList to search for key.
 * @param key string to get its value. ends with \0
 * @returns if found value it will return the pointer of the value. else return NULL
 */
void *getValueByKey(List *map, char *key)
{
    Node *n;
    n = getNodeByKey(map, key);
    if (n == NULL)
        return NULL;
    return n->value;
}

/*
 * get a node pointer by key. return first place it was found.
 * map - List to search for the node in.
 * key - String to search for.
 * return NULL if not found, else return pointer to the node.
 */
Node *getNodeByKey(List *map, char *key)
{
    Node *cur = map->head;

    while (cur != NULL)
    {
        if (strcmp(cur->key, key) == 0)
        {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}

/*
    add key, value node to map. return ERROR_LABEL_DEFINED if such key already exists (this function wont allow duplicate value).
    return - status code.
*/
int addToMapNoDuplicate(List *map, char *key, void *value)
{
    if (exists(map, key) == TRUE)
        return ERROR_LABEL_DEFINED;
    return addToList(map, key, value);
}
