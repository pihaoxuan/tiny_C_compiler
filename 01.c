#include <stdio.h>
#include <stdlib.h>
// #include <memory.h>
#include <string.h>
#include <fcntl.h>
#define int long long

int token;
wchar_t *src, *old_src;
int poolsize;
int line;

//read the value of next char's ACSII
void next()
{
    token = *src++;
    return;
}

// pewpwd
void expression(int level)
{
    // do nothing
}

// 循环读取文件，直到token小于0（因为token是ASCII，所以没有小于0 的情况，等于0就是结束符）
void program()
{
    next(); // get next token
    while (token >0)//不能用中文
    {
        printf("token is: %c\n", token);
        next();
    }
}

// 虚拟机接口
int eval()
{ // do nothing yet
    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024; // arbitrary size
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

    program();
    return eval();
}