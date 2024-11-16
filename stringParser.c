#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "data.h"
#define notBlank(c) (c != ' ' && c != '\t' && c != '\n')

/* get nth argument
 * str - line to read from.
 * n - nth argument (number), starts from 1 (first arg).
 * maxWordLen - max length of word inside line (number).
 */
char *getNthArgument(char *str, int n, int maxWordLen)
{
    int i;
    int wordPos = 0;
    int wordCount = 0;
    int inWord = FALSE;
    char *word;
    word = malloc(maxWordLen + 1); /* +1 for \0 */
    if (word == NULL)
        return word;

    for (i = 0; str[i] != '\0' && wordCount <= n; ++i)
    {
        if (notBlank(str[i]))
        {
            if (inWord == FALSE)
                ++wordCount;
            inWord = TRUE;
        }
        else
        {
            inWord = FALSE;
        }
        if (inWord == TRUE && wordCount == n)
        {
            word[wordPos] = str[i];
            ++wordPos;
        }
    }

    word[wordPos] = '\0';
    return word;
}

/*
 * compare nth argument in a string to a word.
 * str - string to get argument from.
 * n - number of argument to get starting from 1.
 * word - string to compare with nth argument.
 * return TRUE if equal, else FALSE.
 */
int isNthArgumentEqual(char *str, int n, char *word)
{
    int len, equal;
    char *nthArg;
    len = strlen(word);
    nthArg = getNthArgument(str, n, len);
    equal = strcmp(nthArg, word);
    free(nthArg);
    return (equal == 0) ? TRUE : FALSE;
}

/*
 * duplicate a string.
 * str - string to duplicate
 * return - copy of the string. (should be freed.)
 */
char *dupStr(char *str)
{
    char *dup;
    int len;

    len = strlen(str);
    dup = malloc(len + 1); /* len + 1 for the \0 */

    if (dup == NULL)
        return NULL;

    memcpy(dup, str, len + 1); /* len + 1 since we want the \0 */
    return dup;
}

/*
 * count arguments in string.
 */
int countArguments(char *str, char split)
{
    int i;
    int inWord = FALSE;
    int count = 0;

    for (i = 0; str[i] != '\0'; ++i)
    {
        if (notBlank(str[i]) && str[i] != split)
        {
            if (inWord == FALSE) /* if its not blank and we are not inside a word */
            {
                count++;
            }
            inWord = TRUE;
        }
        else
        {
            inWord = FALSE;
        }
    }

    return count;
}

/*
 * split line by spaces, tabs, new line (blank chars) and a char. return array of strings (each string is an argument).
 * line - line (string) to split.
 * count - pointer to int, the length of the array returned
 * return - array of strings. SHOULD be freed. (each pointer should be freed and the array also should be freed.)
 */
char **getArguments(char *line, int *count, char split)
{
    int i;
    int wordPos = 0, argPos = 0, argCount;
    int inWord = FALSE;
    char **args;
    char *word;
    args = NULL;

    /* first count how many arguments and get space */
    argCount = countArguments(line, split);
    args = malloc(argCount * sizeof(char *));
    if (args == NULL)
        return NULL;

    for (i = 0; line[i] != '\0' && line[i] != EOF; ++i)
    {
        if (notBlank(line[i]) && line[i] != split)
        {
            if (inWord == FALSE)
            {
                word = malloc(MAX_LINE_LENGTH);
                if (word == NULL)
                    return NULL;
            }
            inWord = TRUE;
            word[wordPos++] = line[i];
        }
        else if (inWord == TRUE)
        {
            inWord = FALSE;
            word[wordPos] = '\0';
            args[argPos++] = word;
            wordPos = 0;
        }
    }
    if (inWord == TRUE)
    {
        word[wordPos] = '\0';
        args[argPos++] = word;
    }

    *count = argCount;
    return args;
}

/*
 * get start index of nth argument (starting from 1).
 * return index if end was not reached. if end was reached before nth arg return -1.
 */
int getNthIndex(char *str, int n)
{
    int inWord = FALSE;
    int wordCount = 0;
    int i;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (notBlank(str[i]))
        {
            if (inWord == FALSE)
            {
                wordCount++;
                if (wordCount == n)
                    return i;
            }
            inWord = TRUE;
        }
        else
        {
            inWord = FALSE;
        }
    }

    return -1;
}

#define isDigit(c) (('0' <= c && c <= '9') || c == '+' || c == '-')

/*
split line (string) by a char.
line - string to split.
split - char to split string each time its written.
numsLen - pointer to int, size of the returned int array.
return - int array of length numsLen.
if a non number was found, numsLen will be -1 and return NULL.
if there was a memory error, return NULL and numsLen will be set to 0.

NOTE: return value should be freed.
*/
int *splitInts(char *line, char split, int *numsLen)
{
    int i;            /* 1 for \0. */
    int *nums, *temp; /* array containing the numbers. */
    int numsCount = 0;
    int inNum = FALSE, expectNum = TRUE;
    int curNumIndex = 0;
    char *curNum;

    curNum = malloc(MAX_LINE_LENGTH);
    if (curNum == NULL)
    {
        *numsLen = 0;
        return NULL;
    }
    nums = malloc(0);

    for (i = 0; line[i] != '\0'; ++i)
    {
        if (line[i] != '\t' && line[i] != ' ')
        {
            if ((line[i] == split || line[i] == '\n') && inNum == TRUE)
            {
                expectNum = TRUE;
                inNum = FALSE;
                temp = realloc(nums, numsCount + 1);
                if (temp == NULL)
                {
                    *numsLen = 0;
                    return NULL;
                }
                nums = temp;

                curNum[curNumIndex] = '\0';
                nums[numsCount++] = atoi(curNum);
                curNumIndex = 0;
            }
            else if (isDigit(line[i]) && expectNum == TRUE)
            {
                inNum = TRUE;
                curNum[curNumIndex++] = line[i];
            }
            else
            {
                *numsLen = -1;
                return NULL;
            }
        }
        else
        {
            if (inNum == TRUE)
            {
                expectNum = FALSE;
            }
            inNum = FALSE;
        }
    }

    if (inNum == TRUE)
    {
        temp = realloc(nums, numsCount + 1);
        if (temp == NULL)
        {
            *numsLen = 0;
            return NULL;
        }
        nums = temp;

        curNum[curNumIndex] = '\0';
        nums[numsCount++] = atoi(curNum);
    }

    free(curNum);
    *numsLen = numsCount;
    return nums;
}

/* get a to the power of b*/
int power(int a, int b)
{
    int i, n = a;

    if (b == 0)
        return 1;
    for (i = 1; i < b; i++)
        n *= a;
    return n;
}

/*
get binary of positive integer including 0. max value is 2 to the power of len-1. CAN OVERFLOW!
*/
char *getBinaryPositive(int n, int len)
{
    char *bin;
    int i, temp, sum = n;

    bin = malloc(len + 1);
    if (bin == NULL)
        return NULL; /* failed to malloc */

    /* / = 1 . = 0 */

    for (i = 0; i < len; i++)
    {
        temp = power(2, len - i - 1);
        if (temp <= sum)
        {
            bin[i] = '/';
            sum -= temp;
        }
        else
            bin[i] = '.';
    }

    bin[len] = '\0';
    return bin;
}

/*
convert int to string binary using 2s complement. CAN OVERFLOW!
max value: 2 to the power of len-2
min value: negative (2 to the power of len-1)
*/
char *getBinary(int n, int len)
{
    char *bin;
    if (n >= 0)
        return getBinaryPositive(n, len);
    bin = getBinaryPositive(n + power(2, (len - 1)), len);
    bin[0] = '/'; /* -2^(len-1) + n + 2^(len-1) = n. n negative -> no overflow */

    return bin;
}

/*
 convert part of binary to int.
 start - index to stop converting the string to int (including this index. ).
 end - index to start converting the binary to number (the converted number includes this index. ).
 return converted value.
 NOTE: bin should be a valid binary string (it wont be checked).
*/
int binToInt(char *bin, int start, int end)
{
    int i;
    int num = 0;
    int curPower = 1;
    for (i = end; i >= start; i--)
    {
        if (bin[i] == '/')
        {
            num += curPower;
        }
        curPower *= 2;
    }
    return num;
}