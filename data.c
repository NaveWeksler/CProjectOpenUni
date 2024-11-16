#include "data.h"
#include <stdio.h>

/*
 * print status code.
 */
void printStatus(int status)
{
    switch (status)
    {
    case ERROR_COULD_NOT_ALLOCATE:
        fprintf(stderr, "Error: Could not allocate.\n");
        break;
    case ERROR_EXPECTED_MORE_ARGUMENTS:
        fprintf(stderr, "Error: Expected more arguments.\n");
        break;
    case ERROR_EXPECTED_LESS_ARGUMENTS:
        fprintf(stderr, "Error: Expected less arguments.\n");
        break;
    case ERROR_NOT_AN_INTEGER:
        fprintf(stderr, "Error: Not an integer\n");
        break;
    case ERROR_NOT_A_STRING:
        fprintf(stderr, "Error: Expected a string.\n");
        break;
    case ERROR_LABEL_DEFINED:
        fprintf(stderr, "Error: Label already defined.\n");
        break;
    case ERROR_INSTRUCTION_DOES_NOT_EXIST:
        fprintf(stderr, "Error: Instruction does not exist.\n");
        break;
    case ERROR_NOT_A_PARAMETER:
        fprintf(stderr, "Error: Parameter is invalid.\n");
        break;
    case ERROR_INVALID_ADDRESSING_METHOD:
        fprintf(stderr, "Error: Invalid addressing method for this instruction.\n");
        break;
    case ERROR_LABEL_DOES_NOT_EXIST:
        fprintf(stderr, "Error: Label does not exist.\n");
        break;
    }
}

/*
 * print line and status code.
 */
void printLineStatus(int line, int status)
{
    fprintf(stderr, "Line %d, ", line);
    printStatus(status);
}
