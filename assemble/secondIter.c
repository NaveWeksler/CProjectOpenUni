#include <stdio.h>
#include "../data.h"
#include "../dataTypes/linkedList.h"
#include <string.h>
#include "./util.h"
#include "../stringParser.h"
#include "../dataTypes/map.h"
#include <stdlib.h>
#include "operationParser.h"

void tempPrintMem2(char **memBin, int len)
{
    int i;
    printf("start:\n");
    for (i = 0; i < len; i++)
    {
        printf("%d: %s\n", i, memBin[i]);
    }
    printf("end\n");
}

/*
 * save entry data to file.
 * entry - list of all entries (string: int, label name: address)
 * filename - string of the file to save extern symbols in.
 * return status code.
 */
int saveEntryData(List *entry, char *filename)
{
    FILE *file;
    Node *cur;
    cur = entry->head;
    file = fopen(filename, "w");
    if (!file)
        return ERROR_COULD_NOT_OPEN_FILE;
    while (cur != NULL)
    {
        fprintf(file, "%s %d\n", cur->key, ((SymbolNodeVal *)cur->value)->value); /* entry is list of string: SymbolNodeVal since the value is from symbolTable.  */
        cur = cur->next;
    }
    fclose(file);
    return SUCCESS;
}

/*
 * save extern data to file.
 * externData - list of extern label and where they are used (list of string: int, label name: address used in. multiple of the same name can be used.)
 */
int saveExternData(List *externData, char *filename)
{
    FILE *file;
    Node *cur;
    cur = externData->head;
    file = fopen(filename, "w");
    if (!file)
        return ERROR_COULD_NOT_OPEN_FILE;
    while (cur != NULL)
    {
        fprintf(file, "%s %d\n", cur->key, *(int *)cur->value);
        cur = cur->next;
    }
    fclose(file);
    return SUCCESS;
}

/*
 * free the externData list.
 */
void freeExternData(List data)
{
    Node *temp;
    Node *cur = data.head;

    while (cur != NULL)
    {
        temp = cur;
        cur = cur->next;
        free(temp->key);
        free(temp->value); /* value is int pointer we allocated. */
        free(temp);
    }
}

/*
 * free the entrySymbols list
 */
void freeEntrySymbols(List data)
{
    Node *temp;
    Node *cur = data.head;

    while (cur != NULL)
    {
        temp = cur;
        cur = cur->next;
        free(temp->key);
        free(temp);
        /* we dont need to free value since its pointer from symbolTable. (which will be freed later.) */
    }
}

/*
 * second iteration.
 * from - file to read from (the macro parsed file).
 * symbolTable - the list of string: int, symbol: address.
 * dataList - pointer to the dataList (array of strings, each string is binary of data )
 * instructionList - pointer to the instructionList (array of string, each string is binary of an instruction or null pointer to be replaced with a binary word.)
 * entryFilename - string, name of the file to save entry data in.
 * externFilename - string, name of the file to save extern data in.
 */
int secondIter(FILE *from, List *symbolTable, char ***dataList, char ***instructionsList, char *entryFilename, char *externFilename)
{
    List entrySymbols; /* all entry symbols. map of string: string. if no errors were found, add those symbols with their pointer to the .ent file. */
    List externData;   /* all line pos where an extern label was used. */
    int IC = 0, argCount;
    char *line;
    char **args;

    int status, lineCount = 1;

    int foundError = FALSE;

    entrySymbols.head = NULL;
    entrySymbols.tail = NULL;
    externData.head = NULL;
    externData.tail = NULL;

    line = malloc(MAX_LINE_LENGTH);
    if (line == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    while (fgets(line, MAX_LINE_LENGTH, from) != NULL) /* read until the end of the file. */
    {
        args = getArguments(line, &argCount, '\0'); /* since \0 is the end of the string, split only by white spaces. */
        printf("line: %s , IC: %d, args: %d , line: %d\n", line, IC, argCount, lineCount);

        if (args == NULL)
        {
            printStatus(ERROR_COULD_NOT_ALLOCATE);
            return ERROR_COULD_NOT_ALLOCATE;
        }
        if (argCount != 0 && line[0] != ';') /* if the line is not blank and not a comment. */
        {
            if (isSymbol(args[0]) == TRUE)
            {
                if (argCount >= 2 && strcmp(args[1], ".entry") == 0)
                {
                    if (argCount < 3)
                    {
                        printLineStatus(lineCount, ERROR_EXPECTED_MORE_ARGUMENTS);
                        foundError = TRUE;
                    }
                    else if (argCount > 3)
                    {
                        printLineStatus(lineCount, ERROR_EXPECTED_LESS_ARGUMENTS);
                        foundError = TRUE;
                    }
                    else
                    {
                        status = addToList(&entrySymbols, dupStr(args[2]), getValueByKey(symbolTable, args[2]));
                        if (status != SUCCESS)
                        {
                            printLineStatus(lineCount, status);
                            foundError = TRUE;
                        }
                    }
                }
                else if (argCount >= 2 && strcmp(args[1], ".extern") != 0 && strcmp(args[1], ".data") != 0 && strcmp(args[1], ".string") != 0)
                {
                    status = handleOtherWords((*instructionsList)[IC], line + getNthIndex(line, 3), instructionsList, &IC, symbolTable, &externData, lineCount);
                    if (status == TRUE)
                    {
                        foundError = TRUE; /* errors are printed inside handleOtherWords*/
                    }
                }
            }
            else
            {
                if (strcmp(args[0], ".entry") == 0)
                {
                    if (argCount < 2)
                    {
                        printLineStatus(lineCount, ERROR_EXPECTED_MORE_ARGUMENTS);
                        foundError = TRUE;
                    }
                    else if (argCount > 2)
                    {
                        printLineStatus(lineCount, ERROR_EXPECTED_LESS_ARGUMENTS);
                        foundError = TRUE;
                    }
                    else
                    {
                        status = addToList(&entrySymbols, dupStr(args[1]), getValueByKey(symbolTable, args[1]));
                        if (status != SUCCESS)
                        {
                            printLineStatus(lineCount, status);
                            foundError = TRUE;
                        }
                    }
                }
                else if (strcmp(args[0], ".extern") != 0 && strcmp(args[0], ".data") != 0 && strcmp(args[0], ".string") != 0)
                {
                    status = handleOtherWords((*instructionsList)[IC], line + getNthIndex(line, 2), instructionsList, &IC, symbolTable, &externData, lineCount);
                    if (status == TRUE)
                    {
                        foundError = TRUE; /* errors are printed inside handleOtherWords*/
                    }
                }
            }
        }

        freeArgs(args, argCount);
        lineCount++;
    }

    free(line);
    if (foundError == TRUE)
    {
        printf("Found errors.\n");
        return ERROR_EXIT;
    }

    printf("done second iter.\n");

    tempPrintMem2(*instructionsList, IC);

    /* done. now we need to save in files. */
    if (entrySymbols.head != NULL) /* if we have anything to save, save it. */
    {
        status = saveEntryData(&entrySymbols, entryFilename);
        if (status != SUCCESS)
            return status;
    }
    if (externData.head != NULL) /* if we have anything to save, save it. */
    {
        status = saveExternData(&externData, externFilename);
        if (status != SUCCESS)
            return status;
    }

    freeExternData(externData);
    freeEntrySymbols(entrySymbols);

    return SUCCESS;
}