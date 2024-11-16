#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "data.h"
#include "./preAssemble/macro.h"
#include "./assemble/assemble.h"

char *concatStrings(char *str1, char *str2, int str2Len)
{
    int str1Len = strlen(str1);
    char *newStr = (char *)malloc(str1Len + str2Len);
    if (newStr == NULL)
        return NULL;
    memcpy(newStr, str1, str1Len);
    memcpy(newStr + str1Len, str2, str2Len); /* copy str2 from the end of str1 (we use pointer arithmatic)*/
    return newStr;
}

/*
 */
int assembleFile(char *filename)
{
    /*
        here we will create the preAsseble file, do the first and second iteration of the assembly.
    */
    char *macroParsedFilename, *assemblyFilename, *machineFilename, *entryFilename, *externFilename;
    int status;

    macroParsedFilename = concatStrings(filename, ".am", 3);
    assemblyFilename = concatStrings(filename, ".as", 3);
    machineFilename = concatStrings(filename, ".ob", 3);
    entryFilename = concatStrings(filename, ".ent", 4);
    externFilename = concatStrings(filename, ".ext", 4);

    if (macroParsedFilename == NULL || assemblyFilename == NULL || machineFilename == NULL || entryFilename == NULL || externFilename == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    status = parseMacros(assemblyFilename, macroParsedFilename);
    printf("parse status: %d\n", status);
    if (status != SUCCESS)
        return status;
    assemble(macroParsedFilename, machineFilename, entryFilename, externFilename);

    free(macroParsedFilename);
    free(assemblyFilename);
    free(machineFilename);
    free(entryFilename);
    free(externFilename);

    return status;
}

int main(int argc, char **argv)
{
    int i;

    if (argc < 2) /* argc includes the call for the assembler so it starts from 1*/
    {
        printf("Error: Expected at least one argument. (file to compile)\n");
        return -1;
    }
    for (i = 1; i < argc; i++) /* for each file name */
    {
        assembleFile(argv[i]); /* assemble the file (parse macros, assemble. )*/
        /* if (status == ERROR_COULD_NOT_ALLOCATE) return -1;  this is a special case. we cant get dynamic space */
    }

    return 0;
}