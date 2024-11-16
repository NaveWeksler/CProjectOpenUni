#include "../dataTypes/linkedList.h"
#define RELOCATABLE 2
#define EXTERNAL 1
#define ABSOLUTE 0

typedef struct symbolNodeVal
{
    int value;
    int type;   /* relocatable = 2 , external = 1 */
    int isData; /* FALSE or TRUE*/
} SymbolNodeVal;

int isSymbol(char *);
void freeArgs(char **args, int argCount);