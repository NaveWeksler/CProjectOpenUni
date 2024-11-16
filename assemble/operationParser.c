#include "../data.h"
#include <stdlib.h>
#include "./parse.h"
#include "../stringParser.h"
#include <stdio.h>
#include <string.h>
#include "../dataTypes/linkedList.h"
#include "../dataTypes/map.h"
#include "util.h"

#define ADDRESS_METHOD_NUM 0
#define ADDRESS_METHOD_LABEL 1
#define ADDRESS_METHOD_REG 3
#define ADDRESS_METHOD_JUMP 2

#define ARG_LABEL 1
#define ARG_REG 3
#define ARG_NUM 0

#define TYPE_LABEL 1
#define TYPE_NUM 2
#define TYPE_REG 3

#define validLabelChar(c) ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('1' <= c && c <= '9')
#define validParamChar(c) (validLabelChar(c) || c == '-' || c == '+' || c == '#')

/*
ops is a number.
since the binary is a string,
 the position in binary as defined in the instructions are the opposite,
  this macro converts a end pos from instructions to a starting position as string.
*/
#define getBinaryStartPos(ops) WORD_LEN - 1 - ops

/*
 * check if str is register (like r7, r1)
 * return TRUE if it is, else FALSE.
 */
int isRegister(char *str)
{
    return (str[0] == 'r' && '0' <= str[1] && str[1] <= '7' && str[2] == '\0') ? TRUE : FALSE;
}

/*
 * check if str can be a label.
 * return TRUE if it can be, else FALSE.
 */
int isLabel(char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (!(validLabelChar(str[i])))
            return FALSE;
    }
    return TRUE;
}

/*
 * check if str is direct number (#-1, #4)
 * return TRUE if it is, else FALSE.
 */
int isNumber(char *str)
{
    int i;
    if (str[0] != '#')
        return FALSE;
    for (i = 1; str[i] != '\0'; i++)
        if (!(('0' <= str[i] && str[i] <= '9') || (str[i] == '+' || str[i] == '-')))
            return FALSE;
    return TRUE;
}

/*
 * check if str is params of second method. ('{LABEL NAME}({param1},{param2})')
 * return TRUE if it is, else FALSE.
 */
int isSecondMethod(char *str)
{
    int i;
    int foundStart = FALSE;
    int foundComma = FALSE;

    if (!(validLabelChar(str[0])))
        return FALSE;

    for (i = 0; str[i] != '\0'; ++i)
    {
        if (foundStart == TRUE)
        {
            if (str[i] == ')')
            {
                return ((str[i + 1] == '\0' || str[i + 1] == '\n') && foundComma == TRUE) ? TRUE : FALSE;
            }
            if (str[i] == ',')
            {
                foundComma = TRUE;
                if (!(validParamChar(str[i + 1])))
                    return FALSE;
            }
            else if (!(validParamChar(str[i])))
                return FALSE;
        }
        else if (str[i] == '(')
        {
            foundStart = TRUE;
            if (!(validParamChar(str[i + 1])))
                return FALSE;
        }
        else if (!(validLabelChar(str[i])))
        {
            return FALSE;
        }
    }

    return FALSE;
}

/*
 get parameters from str following second method definition.
 str - string to get params from.
 return - NULL if could not allocate. else array with two pointers to char (two strings).
 return value and two strings should be freed.
*/
char **getParamsSecondMethod(char *str)
{
    char **params;
    char *par;
    int i, parIndex = 0;
    int start;
    int curWord = 0;

    params = malloc(2 * sizeof(char *));
    if (params == NULL)
    {
        return NULL;
    }

    for (i = 0; str[i] != '\0'; i++)
    {
        if ((curWord == 0 && str[i] == ',') || (curWord == 1 && str[i] == ')'))
        {
            par[parIndex] = '\0';
            params[curWord] = par;

            curWord = 1;
            start = FALSE;
            parIndex = 0;
        }

        if (start == TRUE)
        {
            par[parIndex++] = str[i];
        }
        else if ((curWord == 0 && str[i] == '(') || (curWord == 1 && str[i] == ','))
        {
            start = TRUE;
            par = malloc(MAX_LINE_LENGTH);
            if (par == NULL)
            {
                if (params[0] != NULL)
                    free(params[0]);
                free(params);
                return NULL;
            }
        }
    }

    return params;
}

/*
 * handle second method, get the argSrc, argDest from str that is params of instruction which uses addressing method number 2.
 * str - params (string) of the instruction.
 * argSrc - pointer to string which stores the argSrc.
 * argDest - pointer to string which stores the argDest.
 * return status.
 */
int handleSecondMethod(char *str, char **argSrc, char **argDest)
{
    char **params;
    params = getParamsSecondMethod(str);
    if (params == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    *argSrc = params[0];
    *argDest = params[1];
    free(params);
    return SUCCESS;
}

/*
 * handle any method except for 2. split by comma (if there is a comma) and get argSrc, argDest.
 * str - string to get src, dest from.
 * argSrc - pointer to string to store src in.
 * argDest - pointer to string to store dest in.
 * paramCount - pointer to int, the amount of params there are. (0 or 1 or 2, if 0 both src, dest are null. If 2 src and dest are present).
 * return status
 */
int handleSplitMethods(char *str, char **argSrc, char **argDest, int *paramCount)
{
    char **args;
    int argCount;
    args = getArguments(str, &argCount, ',');
    if (args == NULL)
    {
        return ERROR_COULD_NOT_ALLOCATE;
    }

    if (argCount >= 1)
    {
        *argSrc = args[0];
    }

    if (argCount == 2)
    {
        *argDest = args[1];
    }
    else if (argCount >= 2)
    {
        free(args[0]);
        free(args[1]);
        free(args);
        return ERROR_EXPECTED_LESS_ARGUMENTS;
    }

    free(args);
    *paramCount = argCount;
    return SUCCESS;
}

/*
 * get first word (get first word binary).
 * oper - string, the instruction type (like mov, cmp, sub, add)
 * paramStr - string, the params of the instruction (the arguments after the instruction type.)
 * instructionList - pointer to string array, the array of binary instructions.
 * IC - pointer to IC (number, current index in *instructionList).
 */
int handleFirstWord(char *oper, char *paramStr, char ***instructionList, int *IC)
{
    int i;
    int opCode = 0, era = 0, srcMethod = 0, destMethod = 0, firstArg = 0, secondArg = 0;

    char *code;
    char **temp;

    int status;

    int operationIsSecondMethod = FALSE;

    char *argDest = NULL, *argSrc = NULL; /* dest is first arg, src is second. */

    int words = 1, newIC = *IC, paramCount = 0;

    if (isSecondMethod(paramStr) == TRUE) /* the second method is special so first check it. */
    {
        words++; /* label */
        paramCount = 1;
        operationIsSecondMethod = TRUE;
        destMethod = ADDRESS_METHOD_JUMP; /* only one arg so target (dest) is set. */
        status = handleSecondMethod(paramStr, &argSrc, &argDest);
        if (status != SUCCESS)
        {
            return status;
        }
    }
    else
    {
        status = handleSplitMethods(paramStr, &argSrc, &argDest, &paramCount);
        if (argDest == NULL)
        {
            /* we have only one argument. argSrc */
            argDest = argSrc;
            argSrc = NULL;
        }
        if (status != SUCCESS)
        {
            return status;
        }
    }

    if (argSrc != NULL && isRegister(argSrc))
    {
        if (operationIsSecondMethod == TRUE)
        {
            firstArg = ARG_REG;
        }
        else
        {
            srcMethod = ADDRESS_METHOD_REG;
        }
        words++;
    }
    else if (argSrc != NULL && isNumber(argSrc) == TRUE)
    {
        if (operationIsSecondMethod == TRUE)
        {
            firstArg = ARG_NUM;
        }
        else
        {
            srcMethod = ADDRESS_METHOD_NUM;
        }
        words++;
    }
    else if (argSrc != NULL && isLabel(argSrc) == TRUE)
    {
        if (operationIsSecondMethod == TRUE)
        {
            firstArg = ARG_LABEL;
        }
        else
        {
            srcMethod = ADDRESS_METHOD_LABEL;
        }
        words++;
    }
    else if (argSrc != NULL)
    {
        return ERROR_NOT_A_PARAMETER;
    }

    if (argDest != NULL && isRegister(argDest) == TRUE)
    {
        if (operationIsSecondMethod == TRUE)
        {
            secondArg = ARG_REG;
        }
        else
        {
            destMethod = ADDRESS_METHOD_REG;
        }
        if (argSrc == NULL || isRegister(argSrc) == FALSE) /* check if argSrc is not a reg*/
        {
            words++;
        }
    }
    else if (argDest != NULL && isNumber(argDest) == TRUE)
    {
        if (operationIsSecondMethod == TRUE)
        {
            secondArg = ARG_NUM;
        }
        else
        {
            destMethod = ADDRESS_METHOD_NUM;
        }
        words++;
    }
    else if (argDest != NULL && isLabel(argDest) == TRUE)
    {
        if (operationIsSecondMethod == TRUE)
        {
            secondArg = ARG_LABEL;
        }
        else
        {
            destMethod = ADDRESS_METHOD_LABEL;
        }
        words++;
    }
    else if (argDest != NULL)
    {
        return ERROR_NOT_A_PARAMETER;
    }

    /* check for errors. */
    opCode = getOpCode(oper);
    if (opCode == -1)
        return ERROR_INSTRUCTION_DOES_NOT_EXIST;

    paramCount = validParamCount(opCode, paramCount);
    if (paramCount == 1)
        return ERROR_EXPECTED_LESS_ARGUMENTS;
    if (paramCount == -1)
        return ERROR_EXPECTED_MORE_ARGUMENTS;

    if (argSrc != NULL && validFirstMethod(opCode, srcMethod) == FALSE)
    {
        return ERROR_INVALID_ADDRESSING_METHOD;
    }

    if (argDest != NULL && validSecondMethod(opCode, destMethod) == FALSE)
    {
        return ERROR_INVALID_ADDRESSING_METHOD;
    }

    if (argDest != NULL)
        free(argDest);
    if (argSrc != NULL)
        free(argSrc);

    code = getCode(firstArg, secondArg, opCode, srcMethod, destMethod, era);
    newIC += words;
    temp = realloc(*instructionList, newIC * sizeof(char *));
    if (temp == NULL)
        return ERROR_COULD_NOT_ALLOCATE;

    *instructionList = temp;

    temp[*IC] = code;
    for (i = *IC + 1; i < newIC; i++)
    {
        temp[i] = NULL; /* not necessary */
    }

    *IC = newIC;

    return SUCCESS;
}

/*
 * get binary word of a label.
 * node - Node of symbolTable which has the key label.
 * status - pointer to status variable.
 * return binary word if successful else NULL.
 */
char *getLabelWord(Node *node, int *status)
{
    char *word;
    char *temp;

    word = malloc(WORD_LEN + 1);
    if (word == NULL)
    {
        *status = ERROR_COULD_NOT_ALLOCATE;
        return NULL;
    }
    if (node == NULL)
    {
        *status = ERROR_LABEL_DOES_NOT_EXIST;
        return NULL;
    }
    temp = getBinary(((SymbolNodeVal *)node->value)->type, 2);
    if (temp == NULL)
    {
        *status = ERROR_COULD_NOT_ALLOCATE;
        return NULL;
    }
    memcpy(word + WORD_LEN - 1 - 1, temp, 2); /* get era of label. */
    free(temp);
    temp = getBinary(((SymbolNodeVal *)node->value)->value, 12);
    if (temp == NULL)
    {
        *status = ERROR_COULD_NOT_ALLOCATE;
        return NULL;
    }
    printf("label address: %d, %s\n", ((SymbolNodeVal *)node->value)->value, temp);
    memcpy(word + WORD_LEN - 1 - 13, temp, 12);
    free(temp);

    word[WORD_LEN] = '\0';

    return word;
}

/*
 * get binary word a direct number. (#-1, #4)
 */
char *getNumberWord(char *num, int *status)
{
    char *word;
    char *temp;

    word = malloc(WORD_LEN + 1);
    if (word == NULL)
    {
        *status = ERROR_COULD_NOT_ALLOCATE;
        return NULL;
    }

    temp = getBinary(ABSOLUTE, 2); /* get binary of era absolute. */
    if (temp == NULL)
    {
        *status = ERROR_COULD_NOT_ALLOCATE;
        return NULL;
    }
    memcpy(word + WORD_LEN - 1 - 1, temp, 2);
    free(temp);

    temp = getBinary(atoi(num + 1), 12); /* get binary of era absolute. */
    if (temp == NULL)
    {
        *status = ERROR_COULD_NOT_ALLOCATE;
        return NULL;
    }
    memcpy(word + WORD_LEN - 1 - 13, temp, 12);
    word[WORD_LEN] = '\0';
    free(temp);

    return word;
}

/*
 * get binary word of register
 * reg - the register (string)
 * isSrc - TRUE of FALSE. if it is src or not src (dest).
 * return binary word of register.
 */
char *getRegWord(char *reg, int isSrc)
{
    char *word;
    char *temp;
    int i;
    word = malloc(WORD_LEN + 1);
    if (word == NULL)
        return NULL;

    for (i = 0; i < WORD_LEN; i++)
        word[i] = '.';

    temp = getBinary(ABSOLUTE, 2);
    if (temp == NULL)
        return NULL;
    memcpy(word + getBinaryStartPos(1), temp, 2);
    free(temp);

    if (isSrc == FALSE)
    {
        temp = getBinary(atoi(reg + 1), 6);
        if (temp == NULL)
            return NULL;
        memcpy(word + getBinaryStartPos(7), temp, 6); /* copy binary number of reg in bits 2 - 7.*/
        free(temp);
    }
    else
    {
        temp = getBinary(atoi(reg + 1), 6);
        if (temp == NULL)
            return NULL;
        memcpy(word + getBinaryStartPos(13), temp, 6); /* copy reg number to bits 8 - 13 */
        free(temp);
    }
    /* 2 - 7 */
    word[WORD_LEN] = '\0';
    return word;
}

/*
 * get binary word of instruction with two registers.
 * reg1 - string, src reg.
 * reg2 - string, dest reg.
 */

char *getDoubleRegWord(char *reg1, char *reg2)
{
    char *word;
    char *temp;
    int i;

    word = malloc(WORD_LEN + 1);
    if (word == NULL)
        return NULL;

    for (i = 0; i < WORD_LEN; i++)
        word[i] = '.';

    temp = getBinary(ABSOLUTE, 2);
    if (temp == NULL)
        return NULL;
    memcpy(word + getBinaryStartPos(1), temp, 2); /* copy are to bits 0 - 1 */
    free(temp);

    temp = getBinary(atoi(reg2 + 1), 6);
    if (temp == NULL)
        return NULL;
    memcpy(word + getBinaryStartPos(7), temp, 6); /* copy reg1 number to bits 2 - 7 */
    free(temp);

    temp = getBinary(atoi(reg1 + 1), 6);
    if (temp == NULL)
        return NULL;
    memcpy(word + getBinaryStartPos(13), temp, 6); /* copy reg2 number to bits 8 - 13 */
    free(temp);

    word[WORD_LEN] = '\0';
    return word;
}

/*
 * convert arg code to address code (convert the first or second method to address method. bits 10 - 13 are first and second method. )
 */
int argToAddress(int arg)
{
    switch (arg)
    {
    case ARG_LABEL:
        return ADDRESS_METHOD_LABEL;
    case ARG_NUM:
        return ADDRESS_METHOD_NUM;
    case ARG_REG:
        return ADDRESS_METHOD_REG;
    }
    return 0; /* should not reach here. */
}

/*
 * add extern data to the extern list.
 * externData - the list to add to.
 * wordPos - the current position in instructionList (IC of current word).
 * symbol - string, the symbol to add.
 */
int addExternData(List *externData, int wordPos, char *symbol)
{
    int *pos;
    char *dup;
    int i;

    pos = malloc(sizeof(int));
    dup = malloc(MAX_LABEL_LEN);
    if (pos == NULL || dup == NULL)
        return ERROR_COULD_NOT_ALLOCATE;
    *pos = wordPos + MEM_START_POS;

    for (i = 0; symbol[i] != '\0'; i++)
        dup[i] = symbol[i];

    dup[i] = '\0';

    return addToList(externData, dup, (void *)pos);
}

/*
 * get binary word of the LABEL in second method ("{instruction type} {label name}({first arg},{second arg})", convert {label name} to binary as a label)
 *
 */
char *getSecondMethodLabel(List *symbolTable, char *paramStr, int *status, List *externData, int wordPos)
{
    Node *node;
    char *word;
    char *temp;
    int i;

    temp = malloc(MAX_LINE_LENGTH);
    if (temp == NULL)
    {
        *status = ERROR_COULD_NOT_ALLOCATE;
        return NULL;
    }
    for (i = 0; paramStr[i] != '('; i++)
    {
        temp[i] = paramStr[i];
    }
    temp[i] = '\0';

    node = getNodeByKey(symbolTable, temp);

    if (node != NULL && ((SymbolNodeVal *)(node->value))->type == EXTERNAL)
    {
        *status = addExternData(externData, wordPos, temp);
        if (*status != SUCCESS)
        {
            return NULL;
            free(temp);
        }
    }

    word = getLabelWord(node, status); /* word can be NULL, return word and status so main function will handle the error (if there is an error.)*/

    free(temp);
    return word;
}

/*
 * get binary words of an instruction (starting from second word)
 * firstWord - string, the binary of the first word (we use this to get the rest of the binary words).
 * paramStr - string, the params of the instruction.
 * instructionList - pointer to string array, store the binary instructions.
 * IC - pointer to number, the current position in *instructionList
 * symbolTable - The list of string: int (label: address).
 * externData - List of extern labels and where they are used.
 * return - TRUE if found errors, else FALSE.
 */
int handleOtherWords(char *firstWord, char *paramStr, char ***instructionList, int *IC, List *symbolTable, List *externData, int curLine)
{
    int srcMethod, destMethod;
    Node *node;
    int status = SUCCESS;

    char *word;

    int isFirstMethodReg = FALSE;

    char *argDest = NULL, *argSrc = NULL; /* dest is first arg, src is second. */

    int words = 1, curWordPos = (*IC) + 1, paramCount = 0;

    int foundErrors = FALSE;

    /*
      it is possible that multiple errors are found during this function (1 to 4 errors). Im not sure what I should do in case multiple errors are found in the same line,
      so I decided to print each error. It can easily be converted to print only one error by removing the call to printLineStatus if foundErrors is TRUE already.
      - NOTE: we continue with this function even if we found errors so we know the next binary position for the next line from the file.
   */

    if (isSecondMethod(paramStr) == TRUE) /* the second method is special so first check it. */
    {
        words++; /* label */
        status = handleSecondMethod(paramStr, &argSrc, &argDest);
        if (status != SUCCESS)
        {
            printLineStatus(curLine, status);
            foundErrors = TRUE;
        }
    }
    else
    {
        status = handleSplitMethods(paramStr, &argSrc, &argDest, &paramCount);
        if (status != SUCCESS)
        {
            printLineStatus(curLine, status);
            foundErrors = TRUE;
        }
    }

    if (argSrc == NULL)
    {
        (*IC)++;      /* move to next instruction*/
        return FALSE; /* no arguments so no need to continue. */
    }

    if (argDest == NULL)
    {
        /* only one arg so switch between dest ad src*/
        argDest = argSrc;
        argSrc = NULL;
    }

    /*
    first word should not be null if the program works correctly,
    if we get null as firstWord we did not calculate IC correctly and we cant continue (and we will get a segmentation fault here.)
    so we dont need to check if its null.
    */
    destMethod = binToInt(firstWord, getBinaryStartPos(3), getBinaryStartPos(2)); /* get bits 2 3 as int from first word. */
    srcMethod = binToInt(firstWord, getBinaryStartPos(5), getBinaryStartPos(4));  /* get bits 4 5 as int from first word. */

    if (destMethod == ADDRESS_METHOD_JUMP)
    {
        /*
        we use getBinaryStartPos(13), getBinaryStartPos(12) (last bit to first bit) since in the real string (binary) we start from getBinaryStartPos(13) = 0
        and move to getBinaryStartPos(12) = 1
        */
        srcMethod = binToInt(firstWord, getBinaryStartPos(13), getBinaryStartPos(12));  /* get bits 12 13 as int from first word. */
        destMethod = binToInt(firstWord, getBinaryStartPos(11), getBinaryStartPos(10)); /* get bits 10 12 as int from first word. */
        /* convert arg to address */
        srcMethod = argToAddress(srcMethod);
        destMethod = argToAddress(destMethod);

        word = getSecondMethodLabel(symbolTable, paramStr, &status, externData, curWordPos);

        if (word != NULL)
        {
            (*instructionList)[curWordPos] = word;
        }
        else
        {
            foundErrors = TRUE;
            printLineStatus(curLine, status);
        }

        curWordPos++; /* we move curWordPos forward in order to count where we should be even if we have an error. (so we can catch more errors.) */
    }

    printf("src: %s : %d, dest: %s : %d\n", argSrc, srcMethod, argDest, destMethod);

    if (argSrc != NULL && srcMethod == ADDRESS_METHOD_NUM)
    {
        word = getNumberWord(argSrc, &status);
        if (word != NULL)
        {
            (*instructionList)[curWordPos] = word;
        }
        else
        {
            foundErrors = TRUE;
            printLineStatus(curLine, status);
        }
        curWordPos++; /* we move curWordPos forward in order to count where we should be even if we have an error. (so we can catch more errors.) */
    }
    else if (argSrc != NULL && srcMethod == ADDRESS_METHOD_LABEL)
    {
        node = getNodeByKey(symbolTable, argSrc);
        if (node != NULL && ((SymbolNodeVal *)(node->value))->type == EXTERNAL)
        {
            status = addExternData(externData, curWordPos, argSrc);
            if (status != SUCCESS)
            {
                printLineStatus(curLine, status);
                foundErrors = TRUE;
            }
        }
        word = getLabelWord(node, &status);
        if (word != NULL)
        {
            (*instructionList)[curWordPos] = word; /* add word to instruction list */
        }
        else
        {
            printLineStatus(curLine, status);
            foundErrors = TRUE;
        }
        curWordPos++; /* we move curWordPos forward in order to count where we should be even if we have an error. (so we can catch more errors.) */
    }
    else if (argSrc != NULL && srcMethod == ADDRESS_METHOD_REG)
    {
        isFirstMethodReg = TRUE;
        if (argSrc != NULL && destMethod == ADDRESS_METHOD_REG)
        {
            /* both dest, src are reg, so one word will contain both. */
            word = getDoubleRegWord(argSrc, argDest);
            if (word != NULL)
            {
                (*instructionList)[curWordPos] = word;
            }
            else
            {
                printLineStatus(curLine, status);
                foundErrors = TRUE;
            }
            curWordPos++; /* we move curWordPos forward in order to count where we should be even if we have an error. (so we can catch more errors.) */
        }
        else
        {
            word = getRegWord(argSrc, TRUE);
            if (word != NULL)
            {
                (*instructionList)[curWordPos] = word;
            }
            else
            {
                printLineStatus(curLine, status);
                foundErrors = TRUE;
            }
            curWordPos++; /* we move curWordPos forward in order to count where we should be even if we have an error. (so we can catch more errors.) */
        }
    }

    if (argDest != NULL && destMethod == ADDRESS_METHOD_NUM)
    {
        word = getNumberWord(argDest, &status);
        if (word != NULL)
        {
            (*instructionList)[curWordPos] = word;
        }
        else
        {
            printLineStatus(curLine, status);
            foundErrors = TRUE;
        }
        curWordPos++; /* we move curWordPos forward in order to count where we should be even if we have an error. (so we can catch more errors.) */
    }
    else if (argDest != NULL && destMethod == ADDRESS_METHOD_LABEL)
    {
        node = getNodeByKey(symbolTable, argDest);
        if (node != NULL && ((SymbolNodeVal *)(node->value))->type == EXTERNAL)
        {
            status = addExternData(externData, curWordPos, argDest);
            if (status != SUCCESS)
            {
                printLineStatus(curLine, status);
                foundErrors = TRUE;
            }
        }
        word = getLabelWord(node, &status);
        if (word != NULL)
        {
            (*instructionList)[curWordPos] = word;
        }
        else
        {
            printLineStatus(curLine, status);
            foundErrors = TRUE;
        }
        curWordPos++; /* we move curWordPos forward in order to count where we should be even if we have an error. (so we can catch more errors.) */
    }
    else if (argDest != NULL && destMethod == ADDRESS_METHOD_REG && isFirstMethodReg == FALSE)
    {
        word = getRegWord(argDest, FALSE);
        if (word != NULL)
        {
            (*instructionList)[curWordPos] = word;
        }
        else
        {
            printLineStatus(curLine, status);
            foundErrors = TRUE;
        }
        curWordPos++; /* we move curWordPos forward in order to count where we should be even if we have an error. (so we can catch more errors.) */
    }

    *IC = curWordPos;
    if (argDest != NULL)
        free(argDest);
    if (argSrc != NULL)
        free(argSrc);
    return foundErrors;
}