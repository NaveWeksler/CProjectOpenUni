#include "assemble.h"
#include "firstIter.h"
#include <stdio.h>
#include "../data.h"
#include <stdlib.h>
#include "../dataTypes/linkedList.h"
#include "secondIter.h"
#include "util.h"

#define ADDRESS_LEN 4

/*
 * convert a number to a 4 digit string of the number (12 -> 0012, 102 -> 0102)
 * buf - 5*sizeof(char) bytes of memory to store the num string.
 * num - number to convert to string.
 */
void getStringPos(char *buf, int num)
{
    if (num < 10)
    {
        sprintf(buf, "000%d", num);
    }
    else if (num < 100)
    {
        sprintf(buf, "00%d", num);
    }
    else if (num < 1000)
    {
        sprintf(buf, "0%d", num);
    }
    else
    {
        sprintf(buf, "%d", num);
    }
}

/*
 * save the result of first and second iteration (the machine code)
 * objectFilename - string, the name of the file to save the instructions in.
 * IC - number, the instruction counter.
 * DC - number, the data counter.
 * return - status code.
 */
int saveBinaryInstructions(char **dataList, char **instructionsList, char *objectFilename, int IC, int DC)
{
    FILE *objectFile;
    char *curPosWord;
    int i;

    objectFile = fopen(objectFilename, "w");

    if (!objectFile)
    {
        printf("Error: Cant open %s\n", objectFilename);
        return ERROR_COULD_NOT_OPEN_FILE;
    }

    curPosWord = malloc(sizeof(char) * (ADDRESS_LEN + 1));
    if (curPosWord == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    for (i = 0; i < IC; i++)
    {
        getStringPos(curPosWord, i + MEM_START_POS);
        fprintf(objectFile, "%s %s\n", curPosWord, instructionsList[i]);
    }

    for (i = 0; i < DC; i++)
    {
        getStringPos(curPosWord, IC + i + MEM_START_POS);
        fprintf(objectFile, "%s %s\n", curPosWord, dataList[i]);
    }
    free(curPosWord);
    fclose(objectFile);
    return SUCCESS;
}

/*
 * free instructionList or dataList.
 * len - length of the array.
 */
void freeBinaryList(char **list, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        if (list[i] != NULL)
        { /* this can happen if we stop after first iteration. */
            free(list[i]);
        }
    }
    free(list);
}

void freeSymbolTable(List table)
{
    Node *temp;
    Node *cur = table.head;

    while (cur != NULL)
    {
        temp = cur;
        cur = cur->next;
        free(temp->value); /* free the SymbolNodeVal */
        free(temp->key);
        free(temp);
    }
}

/*
 * assemble. get parsed file name and assemble it.
 * parsedFilename - string, name of the parsed file.
 * objectFilename - string, name of the object file that will be created (if there are no errors.)
 * entryFilename - string, name of the entry file that will be created (if there are no errors and a '.entry' exists in the assembly file)
 * externFilename - string, name of the extern file that will be created (if there are no errors and an extern label was used. )
 *
 */
int assemble(char *parsedFilename, char *objectFilename, char *entryFilename, char *externFilename)
{
    List symbolTable;               /* list of string: int, symbolNodeVal (symbolNodeVal is defined in util.h). This is list of label name and its address. */
    char **dataList = NULL;         /* array of binary (strings). The data binary */
    char **instructionsList = NULL; /* array of binary (strings). The instructions binary */

    int status;
    FILE *parsedFile;

    int IC = 0, DC = 0;

    symbolTable.head = NULL;
    symbolTable.tail = NULL;

    printf("assembling to file: %s, from: %s\n", objectFilename, parsedFilename);

    parsedFile = fopen(parsedFilename, "r");
    if (!parsedFile)
    {
        printf("Error: Cant open %s\n", parsedFilename);
        return ERROR_COULD_NOT_OPEN_FILE;
    }

    /* call first iteration. if successful, call the second iter  */
    status = firstIter(parsedFile, &symbolTable, &dataList, &instructionsList, &IC, &DC);
    printf("firstIter status: %d\n", status);
    if (status == SUCCESS)
    {
        /* call second iter. if successful, generate object file. (entry, extern files will be generated in secondIter if they are needed.)*/
        rewind(parsedFile); /* clear the position in parsedFile so we can read it again from the start. */
        status = secondIter(parsedFile, &symbolTable, &dataList, &instructionsList, entryFilename, externFilename);
        printf("secondIter status: %d\n", status);
        /* do second iteration. */
        if (status == SUCCESS)
        {
            status = saveBinaryInstructions(dataList, instructionsList, objectFilename, IC, DC);
        }
    }
    fclose(parsedFile);

    if (instructionsList != NULL)
    {
        freeBinaryList(instructionsList, IC);
    }
    if (dataList != NULL)
    {
        freeBinaryList(dataList, DC);
    }
    freeSymbolTable(symbolTable);

    return status;
}