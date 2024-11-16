#include "macro.h"
#include "../dataTypes/map.h"
#include "../dataTypes/linkedList.h"
#include "../data.h"
#include "../stringParser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * parse macros to file.
 * assemblyFilename - string, name of the file to parse.
 * parsedFilename - string, name of the file to parse to.
 * return status code.
 */
int parseMacros(char *assemblyFilename, char *parsedFilename)
{
    FILE *parsedFile;
    FILE *assemblyFile;
    printf("parsing macros to file: %s, from: %s\n", parsedFilename, assemblyFilename);

    assemblyFile = fopen(assemblyFilename, "r");
    if (!assemblyFile)
    {
        printf("Error: File %s does not exist\n", assemblyFilename);
        return ERROR_COULD_NOT_OPEN_FILE;
    }
    parsedFile = fopen(parsedFilename, "w");
    if (!parsedFile)
    {
        printf("Error: Cant open %s\n", parsedFilename);
        return ERROR_COULD_NOT_OPEN_FILE;
    }

    parseMacrosToFile(assemblyFile, parsedFile); /* parse the macros in assemblyFile to parsedFile. */

    fclose(parsedFile);
    fclose(assemblyFile);
    return SUCCESS;
}

/*
 * free all macros after parseMacrosToFile function.
 */
void freeMacroList(List *macros)
{
    Node *cur = macros->head;
    Node *next;

    while (cur != NULL)
    {
        next = cur->next;
        free(cur->value);
        free(cur->key);
        free(cur);
        cur = next;
    }
}

/*
 * parse macros. No need to check errors (In the description of the macro parser it is stated that no errors exsist in macros)
 * @param from - assembly file (.as) to parse macros from (to file "to")
 * @param to - file to parse the assembly file to.
 * @return status code
 */
int parseMacrosToFile(FILE *from, FILE *to)
{
    List macros;

    int inMacro = FALSE;
    char *curMacro;
    int curMacroLen = 0;
    Node *curNode;

    char *val;

    char *line;

    line = malloc(MAX_LINE_LENGTH);

    while (fgets(line, MAX_LINE_LENGTH, from) != NULL) /* we will read line stopping at \n max line length is MAX_LINE_LENGTH. if we cant read we will stop */
    {

        /* we need to search for "mcr" or "endmcr" so we will break the string in each space or \t */
        if (inMacro == FALSE)
        {
            if (isNthArgumentEqual(line, 1, "mcr") == TRUE) /* the first arg is mcr, we have a macro definition */
            {
                inMacro = TRUE;

                /* init curNode (later we will add value and add it to the list )*/
                curNode = createNode();
                if (curNode == NULL)
                    return ERROR_COULD_NOT_FIND_SPACE;

                setKey(curNode, getNthArgument(line, 2, MAX_LINE_LENGTH));

                curMacro = malloc(0); /* just initialize curMacro to prevent realloc error. */
            }
            else /* we dont have a macro definition, we are not in macro */
            {
                val = searchMacros(line, &macros); /* search if a macro call exists in the line */
                if (val == NULL)
                {
                    fprintf(to, "%s", line); /* print line to the macro parsed file. */
                }
                else
                {
                    fprintf(to, "%s", val); /* line has a macro call, print it to the file. */
                }
            }
        }
        else if (isNthArgumentEqual(line, 1, "endmcr") == TRUE) /* inMacro flag is TRUE. if we found the end of the macro ("endmcr"). stop the macro copy process.*/
        {

            setValue(curNode, curMacro);
            addNodeToList(&macros, curNode);
            inMacro = FALSE;

            curMacroLen = 0;
        }
        else /* inside a macro definition */
        {
            /* copy macro line */
            curMacroLen += strlen(line); /* +1 for \0 at the end. */

            /* check if curMacro string has been initialized */

            curMacro = realloc(curMacro, curMacroLen);

            if (curMacro == NULL)
            { /* we check again to make sure we malloc or realloc did not fail. */
                return ERROR_COULD_NOT_FIND_SPACE;
            }

            strcat(curMacro, line);
        }
    }

    free(line);

    /* free all nodes */
    freeMacroList(&macros);

    return SUCCESS;
}

char *searchMacros(char *line, List *map)
{
    char *name;
    char *value;
    name = getNthArgument(line, 1, MAX_LINE_LENGTH); /* get the first arg */
    if (name == NULL)
        return NULL;
    value = (char *)getValueByKey(map, name); /* check if first arg is inside macro map */
    free(name);
    return value; /* can be null. */
}