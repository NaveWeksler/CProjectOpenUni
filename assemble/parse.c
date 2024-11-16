#include "parse.h"
#include <stdlib.h>
#include <string.h>
#include "../data.h"
#include "../stringParser.h"

#define FIRST_ARG_BITS 2
#define SECOND_ARG_BITS 2
#define OPCODE_BITS 4
#define FIRST_METHOD_BITS 2
#define SECOND_METHOD_BITS 2
#define ERA_BITS 2

/**
 * @param word string to convert to opcode.
 * converts word to opcode. return -1 if word is not a command (error).
 */
int getOpCode(char *word)
{

    if (strcmp(word, "mov") == 0)
        return MOV_CODE;
    if (strcmp(word, "cmp") == 0)
        return CMP_CODE;
    if (strcmp(word, "add") == 0)
        return ADD_CODE;
    if (strcmp(word, "sub") == 0)
        return SUB_CODE;
    if (strcmp(word, "not") == 0)
        return NOT_CODE;
    if (strcmp(word, "clr") == 0)
        return CLR_CODE;
    if (strcmp(word, "lea") == 0)
        return LEA_CODE;
    if (strcmp(word, "inc") == 0)
        return INC_CODE;
    if (strcmp(word, "dec") == 0)
        return DEC_CODE;
    if (strcmp(word, "jmp") == 0)
        return JMP_CODE;
    if (strcmp(word, "bne") == 0)
        return BNE_CODE;
    if (strcmp(word, "red") == 0)
        return RED_CODE;
    if (strcmp(word, "prn") == 0)
        return PRN_CODE;
    if (strcmp(word, "jsr") == 0)
        return JSR_CODE;
    if (strcmp(word, "rts") == 0)
        return RTS_CODE;
    if (strcmp(word, "stop") == 0)
        return STOP_CODE;
    /* error: word does not match anything. we will return error */
    return -1;
}

/*
 * util function for 'getCode'. append value of 'bits' bits to code.
 */
int addCode(char *code, int value, int bits)
{
    char *temp;
    temp = getBinary(value, bits);
    if (temp == NULL)
        return ERROR_COULD_NOT_ALLOCATE;
    memcpy(code, temp, bits);
    free(temp);
    return SUCCESS;
}

/*
 * get binary code (string of special base 2) of first word based on its params.
 * return NULL if error else string of the binary code.
 */
char *getCode(int fa, int sa, int opcode, int fm, int sm, int era)
{
    /*  */
    int pos = 0;
    char *code;
    code = malloc(WORD_LEN + 1);
    if (code == NULL)
        return NULL;
    /*
        fa 2 bits
        sa 2 bits
        opcode 4 bits
        fm 2 bits
        sm 2 bits
        era 2 bits
    */

    if (addCode(code + pos, fa, FIRST_ARG_BITS) != SUCCESS)
        return NULL;
    pos += FIRST_ARG_BITS;

    if (addCode(code + pos, sa, SECOND_ARG_BITS) != SUCCESS)
        return NULL;
    pos += SECOND_ARG_BITS;

    if (addCode(code + pos, opcode, OPCODE_BITS) != SUCCESS)
        return NULL;
    pos += OPCODE_BITS;

    if (addCode(code + pos, fm, FIRST_METHOD_BITS) != SUCCESS)
        return NULL;
    pos += FIRST_METHOD_BITS;

    if (addCode(code + pos, sm, SECOND_METHOD_BITS) != SUCCESS)
        return NULL;
    pos += SECOND_METHOD_BITS;

    if (addCode(code + pos, era, ERA_BITS) != SUCCESS)
        return NULL;
    pos += ERA_BITS;

    code[pos] = '\0';

    return code;
}

/**
 * @param flag 'A' or 'R' or 'E'. represents the first 2 bytes in first word.
 * return code for each char
 */
char *getARE(char flag)
{
    if (flag == 'A')
        return ".."; /* absolute */

    if (flag == 'E')
        return "./"; /* external */

    if (flag == 'R')
        return "/."; /* relocatable */

    return NULL;
}

/*
 check if a valid opCode (code of instruction) has the correct amount of params.
 return 0 if correct amount of params, 1 if too many params, -1 if too less params
*/
int validParamCount(int code, int count)
{
    switch (code)
    {
    case MOV_CODE:
    case CMP_CODE:
    case ADD_CODE:
    case SUB_CODE:
    case LEA_CODE:
        if (count > 2)
            return 1;
        if (count < 2)
            return -1;
        return 0;
    case NOT_CODE:
    case CLR_CODE:
    case INC_CODE:
    case DEC_CODE:
    case JMP_CODE:
    case BNE_CODE:
    case RED_CODE:
    case PRN_CODE:
    case JSR_CODE:
        if (count > 1)
            return 1;
        if (count < 1)
            return -1;
        return 0;
    case STOP_CODE:
    case RTS_CODE:
        if (count > 0)
            return 1;
        if (count < 0)
            return -1;
        return 0;
    }

    return 0; /* opcode is not valid, in our use case it does not matter what is the return value since opcode is valid. */
}

/*
 * check if first method is valid for an instruction. ( first method is src method )
 * code - the opcode of the instruction.
 * method - the method to check.
 * return TRUE if valid else FALSE.
 */
int validFirstMethod(int code, int method)
{
    switch (code)
    {
    case MOV_CODE:
    case CMP_CODE:
    case ADD_CODE:
    case SUB_CODE:
        return (0 <= method && method <= 3) ? TRUE : FALSE;
    case LEA_CODE:
        return (method == 1 || method == 2) ? TRUE : FALSE;
    }
    return TRUE;
}

/*
 * check if second method is valid for an instruction. ( second method is dest method )
 * code - the opcode of the instruction.
 * method - the method to check.
 * return TRUE if valid else FALSE.
 */
int validSecondMethod(int code, int method)
{
    switch (code)
    {
    case MOV_CODE:
    case ADD_CODE:
    case SUB_CODE:
    case NOT_CODE:
    case CLR_CODE:
    case LEA_CODE:
    case INC_CODE:
    case DEC_CODE:
    case JMP_CODE:
    case BNE_CODE:
    case RED_CODE:
    case JSR_CODE:
        return (1 <= method && method <= 3) ? TRUE : FALSE;
    case CMP_CODE:
    case PRN_CODE:
        return (0 <= method && method <= 3) ? TRUE : FALSE;
    }
    return TRUE;
}