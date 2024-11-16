#include "firstIter.h"
#include <stdio.h>
#include "../data.h"
#include "../dataTypes/linkedList.h"
#include "../stringParser.h"
#include <string.h>
#include <stdlib.h>
#include "../dataTypes/map.h"
#include "./parse.h"
#include "util.h"
#include "./operationParser.h"

int handleStringInstruction(char *str, char ***dataList, int *DC)
{
    int i;
    int prevDC = *DC;
    char **tempList;

    int len = 0;

    if (str[0] != '"')
        return ERROR_NOT_A_STRING;
    for (i = 1; str[i] != '"' && str[i] != '\0'; i++)
    {
        len++;
    }

    if (str[i] == '\0')
        return ERROR_NOT_A_STRING;
    else if (str[i] == '"' && str[i + 1] != '\0')
        return ERROR_NOT_A_STRING;

    *DC += len + 1;
    tempList = realloc(*dataList, sizeof(int *) * (*DC));
    if (tempList == NULL)
        return ERROR_COULD_NOT_ALLOCATE;
    *dataList = tempList;

    for (i = 0; i < len; i++)
    {
        tempList[prevDC + i] = getBinary(str[i + 1], WORD_LEN);
    }
    tempList[prevDC + i] = getBinary(0, WORD_LEN); /* add terminator at the end.. */

    return SUCCESS;
}

int handleDataInstruction(char *data, char ***dataList, int *DC)
{
    int *dataNums, prevDC;
    int dataLen, i;
    char **tempList;
    prevDC = *DC;
    /*
    data is signed numbers separated by ','. For example: +2,+4,-3,-1,-4
    we will return the amount of numbers in data and set memBin with their binary (2s complement)
    */

    dataNums = splitInts(data, ',', &dataLen);
    if (dataNums == NULL)
    {
        if (dataLen == -1)
        {
            return ERROR_NOT_AN_INTEGER;
        }
        return ERROR_COULD_NOT_ALLOCATE;
    }

    *DC += dataLen;
    tempList = realloc(*dataList, (*DC) * sizeof(char *));

    if (tempList == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    *dataList = tempList;

    for (i = 0; i < dataLen; ++i)
    {
        tempList[prevDC + i] = getBinary(dataNums[i], WORD_LEN);
    }

    free(dataNums);
    return SUCCESS;
}

char *dupSymbol(char *symbol)
{
    int len;
    char *dup;

    len = strlen(symbol);
    dup = malloc(len);
    if (dup == NULL)
    {
        return NULL; /* failed to malloc. */
    }

    memcpy(dup, symbol, len);
    if (dup[len - 1] == ':')
    {
        dup[len - 1] = '\0'; /* we remove the last char ':' which any label has.*/
    }
    else
    {
        dup[len] = '\0';
    }
    return dup;
}

int addSymbol(List *list, char *symbol, int val, int type, int isData)
{
    SymbolNodeVal *nodeVal;
    char *key;
    key = dupSymbol(symbol);
    if (key == NULL)
        return ERROR_COULD_NOT_ALLOCATE;
    nodeVal = malloc(sizeof(SymbolNodeVal));
    if (nodeVal == NULL)
        return ERROR_COULD_NOT_ALLOCATE;
    nodeVal->value = val;
    nodeVal->type = type;
    nodeVal->isData = isData;

    return addToMapNoDuplicate(list, key, (void *)nodeVal);
}

void tempPrintSymbol(List *symbolMem)
{
    Node *node = symbolMem->head;
    printf("start (key: value)\n");
    while (node != NULL)
    {
        printf("%s: %d, type: %d, isData: %d\n", node->key, (int)((SymbolNodeVal *)(node->value))->value, (int)((SymbolNodeVal *)(node->value))->type, (int)((SymbolNodeVal *)(node->value))->isData);
        node = node->next;
    }
    printf("end\n");
}

void tempPrintMem(char **memBin, int len)
{
    int i;
    printf("start:\n");
    for (i = 0; i < len; i++)
    {
        printf("%d: %s\n", i, memBin[i]);
    }
    printf("end\n");
}

void addToDataSymbols(List *symbols, int add)
{
    Node *cur = symbols->head;
    while (cur != NULL)
    {
        if ((int)((SymbolNodeVal *)cur->value)->isData == TRUE && ((int)((SymbolNodeVal *)cur->value)->type) != EXTERNAL)
        {
            ((SymbolNodeVal *)cur->value)->value += add;
        }
        cur = cur->next;
    }
}

void addToNonExternSymbols(List *symbols, int add)
{
    Node *cur = symbols->head;
    while (cur != NULL)
    {
        if (((int)((SymbolNodeVal *)cur->value)->type) != EXTERNAL)
        {
            ((SymbolNodeVal *)cur->value)->value += add;
        }
        cur = cur->next;
    }
}

int firstIter(FILE *from, List *symbolTable, char ***dataList, char ***instructionsList, int *newIC, int *newDC)
{

    int IC = 0, DC = 0, status = SUCCESS, symbolDefinition = FALSE;
    char *line;

    int foundError = FALSE;

    char **args = NULL;
    int argCount = 0;

    int lineCount = 1;

    line = malloc(MAX_LINE_LENGTH);
    if (line == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    while (fgets(line, MAX_LINE_LENGTH, from) != NULL)
    {

        args = getArguments(line, &argCount, '\0'); /* since \0 is the end of the string, split only by white spaces. */
        if (args == NULL)
        {
            printStatus(ERROR_COULD_NOT_ALLOCATE);
            return ERROR_COULD_NOT_ALLOCATE;
        }

        if (line[0] != ';' && argCount != 0) /* if line[0] is ';' then the line is comment. if argCount is 0 the entire line is spaces tabs and \n.*/
        {
            if (isSymbol(args[0]) == TRUE)
            {
                symbolDefinition = TRUE;
            }

            /* check if we have an instruction .data or .string. there are 4 cases:
            1. LABEL: .data ...\n
            2. LABEL: .string "..."\n
            3. .string "..."\n"
            4. .data ...\n

            */
            if (symbolDefinition == TRUE)
            {
                if (argCount >= 3 && strcmp(args[1], ".data") == 0)
                {
                    status = addSymbol(symbolTable, args[0], DC, RELOCATABLE, TRUE);
                    if (status != SUCCESS)
                    {
                        foundError = TRUE;
                        printLineStatus(lineCount, status);
                    }
                    status = handleDataInstruction(line + getNthIndex(line, 3), dataList, &DC);
                    if (status != SUCCESS)
                    {
                        foundError = TRUE;
                        printLineStatus(lineCount, status);
                    }
                }
                else if (argCount == 3 && strcmp(args[1], ".string") == 0)
                {

                    status = addSymbol(symbolTable, args[0], DC, RELOCATABLE, TRUE);
                    if (status != SUCCESS)
                    {
                        foundError = TRUE;
                        printLineStatus(lineCount, status);
                    }
                    status = handleStringInstruction(args[2], dataList, &DC);
                    if (status != SUCCESS)
                    {
                        foundError = TRUE;
                        printLineStatus(lineCount, status);
                    }
                }
                else
                {
                    status = addSymbol(symbolTable, args[0], IC, RELOCATABLE, FALSE);

                    if (status != SUCCESS)
                    {
                        foundError = TRUE;
                        printLineStatus(lineCount, ERROR_LABEL_DEFINED);
                    }
                    if (argCount >= 2) /* if we have more than the symbol in this line (symbol is one arg, the instruction is the rest.)*/
                    {
                        status = handleFirstWord(args[1], line + getNthIndex(line, 3), instructionsList, &IC);
                        if (status != SUCCESS)
                        {
                            foundError = TRUE;
                            printLineStatus(lineCount, status);
                        }
                    }
                }
            }
            else if (argCount >= 2 && strcmp(args[0], ".data") == 0)
            {

                status = handleDataInstruction(line + getNthIndex(line, 2), dataList, &DC);
                if (status != SUCCESS)
                {
                    foundError = TRUE;

                    printLineStatus(lineCount, status);
                }
            }
            else if (argCount == 2 && strcmp(args[0], ".string") == 0)
            {

                status = handleStringInstruction(args[1], dataList, &DC);
                if (status != SUCCESS)
                {
                    foundError = TRUE;

                    printLineStatus(lineCount, status);
                }
            }
            else if (strcmp(args[0], ".extern") == 0)
            {
                if (argCount > 2)
                {
                    foundError = TRUE;
                    printLineStatus(lineCount, ERROR_EXPECTED_LESS_ARGUMENTS);
                }
                else if (argCount < 2)
                {
                    foundError = TRUE;
                    printLineStatus(lineCount, ERROR_EXPECTED_MORE_ARGUMENTS);
                }
                else
                {
                    printf("args1: %s END\n", args[1]);
                    status = addSymbol(symbolTable, args[1], 0, EXTERNAL, TRUE);
                    if (status != SUCCESS)
                    {
                        foundError = TRUE;
                        printLineStatus(lineCount, status);
                    }
                }
            }
            else if (strcmp(args[0], ".entry") != 0)
            {
                status = handleFirstWord(args[0], line + getNthIndex(line, 2), instructionsList, &IC);
                if (status != SUCCESS)
                {
                    foundError = TRUE;
                    printLineStatus(lineCount, status);
                }
            }

            symbolDefinition = FALSE; /* reset */
        }

        lineCount++;
        freeArgs(args, argCount);
    }

    free(line);

    addToDataSymbols(symbolTable, IC);
    addToNonExternSymbols(symbolTable, MEM_START_POS);

    printf("data:\n");
    tempPrintMem(*dataList, DC);
    printf("instructions:\n");

    tempPrintMem(*instructionsList, IC);

    tempPrintSymbol(symbolTable);

    *newIC = IC;
    *newDC = DC;

    if (foundError == TRUE)
    {
        printf("Found Error in first iteration. Should exit.\n");
        return ERROR_EXIT;
    }

    /* free after second iteration or if status is not SUCCESS*/
    return SUCCESS;
}