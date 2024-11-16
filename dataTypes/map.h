#ifndef Map_H
#define Map_H
#define MAP_KEY_EXISTS -1
#include "linkedList.h"

void *getValueByKey(List *, char *);
int addToMapNoDuplicate(List *, char *, void *);
Node *getNodeByKey(List *, char *);

#endif