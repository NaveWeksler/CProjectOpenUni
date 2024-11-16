#define MOV_CODE 0
#define CMP_CODE 1
#define ADD_CODE 2
#define SUB_CODE 3
#define NOT_CODE 4
#define CLR_CODE 5
#define LEA_CODE 6
#define INC_CODE 7
#define DEC_CODE 8
#define JMP_CODE 9
#define BNE_CODE 10
#define RED_CODE 11
#define PRN_CODE 12
#define JSR_CODE 13
#define RTS_CODE 14
#define STOP_CODE 15

char *getARE(char);
int getOpCode(char *);
char *getCode(int, int, int, int, int, int);
int validParamCount(int, int);
int validFirstMethod(int, int);
int validSecondMethod(int, int);