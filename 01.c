#include <stdio.h>
#include <stdlib.h>
// #include <memory.h>
#include <string.h>
#include <fcntl.h>
#define int long long

// 全局数据结构
int token;          // 当前解析的token值
wchar_t *src;       // 源代码指针（使用宽字符处理多语言）
wchar_t *old_src;   // 源代码起始地址备份
int poolsize;       // 内存池大小（默认256KB）
int line;           // 行号计数器

// 词法分析核心函数
void next() {
    token = *src++; // 1. 读取当前字符 2. 指针前进
}

// pewpwd
void expression(int level)
{
    // do nothing
}
// 语法分析入口（当前仅实现字符循环读取）
void program() {
    next(); 
    while (token > 0) { // 主解析循环（0表示字符串终止符）
        printf("token is: %c\n", token); // 调试输出
        next();
    }
}
int eval(){
    //do nothing
    return 0;
}

// 主程序流程
int main(int argc, char **argv) {
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

    if (!(src = old_src = malloc(poolsize)))  //使用calloc初始化内存，代替malloc，初始化内存为0
    // if(!(src = old_src = calloc(poolsize, sizeof(wchar_t))))
    {
        printf("could not malloc(%lld) for source area\n", poolsize);
        return -1;
    }

    // read the source file
    if ((i = read(fd, src, poolsize - 1)) <= 0)
    {
        printf("read() returned %lld\n", i);
        return -1;
    }
    printf("i=%d\n", i);
    src[i] = 0; // add EOF character
    close(fd);

    program();
    return eval();
}