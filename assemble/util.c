#include "../data.h"
#include <stdlib.h>
#include "util.h"
#include "../dataTypes/linkedList.h"

/*
check if first arg in line is symbol (label)
*/
int isSymbol(char *arg)
{
    int i = 0;
    while (arg[i] != '\0')
        i++;
    if (i == 0)
        return FALSE;
    return arg[i - 1] == ':' ? TRUE : FALSE;
}

void freeArgs(char **args, int argCount)
{
    int i;
    for (i = 0; i < argCount; i++)
    {
        free(args[i]);
    }
    free(args);
}
