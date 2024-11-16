#include "parse.h"

/**
 * @param word string to convert to opcode.
 * converts word to opcode. return -1 if word is not a command (error). This is for first word in machine code.
 */
char *getOpcode(char *word)
{
    /* we have to use if statments. switch does not work with string.*/
    if (word == "mov")
        return "...."; /* 0000 in binary. 0 in decimal */
    if (word == "cmp")
        return ".../";
    if (word == "add")
        return "../.";
    if (word == "sub")
        return "..//";
    if (word == "not")
        return "./..";
    if (word == "clr")
        return "././";
    if (word == "lea")
        return ".//.";
    if (word == "inc")
        return ".///";
    if (word == "dec")
        return "/...";
    if (word == "jmp")
        return "/../";
    if (word == "bne")
        return "/./.";
    if (word == "red")
        return "/.//";
    if (word == "prn")
        return "//..";
    if (word == "jsr")
        return "//./";
    if (word == "rts")
        return "///.";
    if (word == "stop")
        return "////";
    /* error: word does not match anything. we will return error */
    return "\0";
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
}