#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#define int long long

int token;
char *src, *old_src;
int poolsize;
int line;

int *text, *old_text, *stack; // text old_text你宻tack瀛樻斁int
char *data;                   // data瀛樻斁char

int *bp, *sp, *pc; // 瀛樻斁鎸囬拡鍦板潃
int ax = 0;        // 瀛樻斁鎸囦护杩愮畻缁撴灉

enum // 鏋氫妇鎸囦护闆嗭紙绠€鍗曟寚浠ら泦锛�
{
    LEA,
    IMM,
    JMP,
    CALL,
    JZ,
    JNZ,
    ENT,
    ADJ,
    LEV,
    LI,
    LC,
    SI,
    SC,
    PUSH,
    OR,
    XOR,
    AND,
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
    SHL,
    SHR,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    OPEN,
    READ,
    CLOS,
    PRTF,
    MALC,
    MSET,
    MCMP,
    EXIT
};

// read the point of src and store the point in token as an integer.
void next()
{
    token = *src++;
    return;
}

// 琛ㄨ揪寮忓垎鏋恜ewpwd
void expression(int level)
{
    // do nothing
}

// 循环读取文件，直到token小于0（因为token是ASCII，所以没有小于0 的情况，等于0就是结束符）
void program()
{
    next();           // get next token
    while (token > 0) // 不能用中文
    {
        printf("token is: %c\n", token);
        next();
    }
}

// 虚拟机接口
int eval()
{
    int op, *tmp;
    while (1)
    {
        if (op == IMM)
        {
            ax = *pc++;
        }
        else if (op == LC)
        {
            ax = *(char *)ax;
        }
        else if (op == LI)
        {
            ax = *(int *)ax;
        }
        else if (op == SC)
        {
            ax = *(char *)*sp++ = ax;
        }
        else if (op == SI)
        {
            *(int *)*sp++ = ax;
        }
        else if (op == PUSH)
        {
            *--sp = ax;
        }
        else if (op == JMP)
        {
            pc = (int *)*pc;
        }
        else if (op == JZ)
        {
            pc = ax ? pc + 1 : (int *)*pc;
        }
        else if (op == JNZ)
        {
            pc = ax ? (int *)*pc : pc + 1;
        }
        else if (op == CALL)
        {
            sp--;
            *sp = (int)(pc + 1);
            pc = (int *)*pc;
        }
        else if (op == ENT)
        {
            *--sp = (int)bp;
            bp = sp;
            sp = sp - *pc++;
        }
        else if (op == ADJ)
        {
            sp = sp + *pc++;
        }
        else if (op == LEV)
        {
            sp = bp;
            bp = (int *)*sp++;
            pc = (int *)*sp++;
        }
        else if (op == LEA)
        {
            ax = (int)(bp + *pc++);
        }
        else if (op == JNZ)
        {
            pc = ax ? (int *)*pc : pc + 1;
        }
        else if (op == JNZ)
        {
            pc = ax ? (int *)*pc : pc + 1;
        }
        else if (op == JNZ)
        {
            pc = ax ? (int *)*pc : pc + 1;
        }
        else if (op == JNZ)
        {
            pc = ax ? (int *)*pc : pc + 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;

    argc--;
    argv++;

    poolsize = 1024 * 1024; // arbitrary size
    line = 1;

    if ((fd = open(*argv, 0)) < 0)
    {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize)))
    {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read the source file
    if ((i = read(fd, src, poolsize - 1)) <= 0)
    {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    close(fd);

    if (!(text = old_text = malloc(poolsize)))
    {
        printf("COULD NOT MALLOC(%d) FOR TEXT AREA.\n", poolsize);
        return -1;
    }
    if (!(data = malloc(poolsize)))
    {
        printf("could not malloc(%d) for data area.\n", poolsize);
        return -1;
    }
    if (!(stack = malloc(poolsize)))
    {
        printf("could not malloc(%d) for stack area.\n");
        return -1;
    }

    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    bp = sp = (int *)((int)stack + poolsize);
    ax = 0;

    program();
    printf("1");
    return eval();
}