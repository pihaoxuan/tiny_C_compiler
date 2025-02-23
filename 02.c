#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#define int long long

int token;
char *src = 0;
char *old_src = 0;
int poolsize;
int line;

// 初始化text、old_text、stack、data
int *text = 0;
int  *old_text = 0;
int *stack = 0;
char *data = 0;

//初始化bp基址寄存器指针、sp指向栈顶元素的指针寄存器、pc程序计数器、ax通用寄存器
int *bp, *sp, *pc;
int ax = 0;
int cycle = 0;

//枚举指令集
enum
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

void expression(int level)
{
    // do nothing
}

// 循环读取文件，直到token小于0（因为token是ASCII，所以没有小于0 的情况，等于0就是结束符）
void program() {
    next();                  // get next token
    // while (token > 0)    //这样不能读取中文，因为中文的ASCII码大于0，转换成token的时候可能为负值
    while (token != 0)      //改为token!=0
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
        op = *pc++; //初始化op为当前指令
        if (op == IMM)  //将立即数载入ax
        {
            ax = *pc++;
        }
        else if (op == LC)  //将对应地址中的字符载入ax中，要求ax中存放地址
        {
            ax = *(char *)ax;
        }
        else if (op == LI)  //将对应地址中的整数载入ax中，要求ax中存放地址
        {
            ax = *(int *)ax;
        }
        else if (op == SC)  //将ax中的数据作为字符存入对应地址中，要求栈顶存放地址
        {
            ax = *(char *)*sp++ = ax;    //原写法
            //新写法
            // *sp = ax;
            // ax = *(char *)*sp;
            // sp++;


        }
        else if (op == SI)  //将ax中的数据作为整数存入对应地址中，要求栈顶存放地址
        {
            *(int *)*sp++ = ax; //原写法
            //新写法
            // *sp = ax;
            // ax = *(int *)*sp;
            // sp++;
        }
        else if (op == PUSH)    //将ax中的数据压入栈中
        {
            // *--sp = ax;  //原写法，太炫了，改成下面的写法
            sp--;
            *sp = ax;
        }
        else if (op == JMP)
        {
            pc = (int *)*pc;
        }
        else if (op == JZ)  //如果ax为0，跳转到对应地址，否则继续执行下一条指令
        {
            pc = ax ? pc + 1 : (int *)*pc;
        }
        else if (op == JNZ) //如果ax不为0，跳转到对应地址，否则继续执行下一条指令
        {
            pc = ax ? (int *)*pc : pc + 1;
        }
        else if (op == CALL)    //把现在在执行的指令的下一条压栈，然后转到子函数执行
        {
            sp--;
            *sp = (int)(pc + 1);
            pc = (int *)*pc;
        }
        else if (op == LEV) 
        {
            //将bp指向 原来指向的内容 所指向的内容，然后将这块空间出栈
            sp = bp;
            bp = (int *)*sp;
            sp++;
            
            //从子函数返回
            pc = (int *)*sp;
            sp++;
        }
        else if (op == ENT)
        {
            sp--;
            *sp = (int)bp;    //把bp这个地址作为数据压栈，即保留bp数据
            bp = sp;
            sp = sp - *pc;    //在栈上保留*pc这么多空间
            pc++;             //pc指向下一条指令
        }
        else if (op == ADJ) //将调用子函数时压栈的数据清除
        {
            //回收*pc这么多空间，然后pc指向下一条指令
            sp = sp + *pc;
            pc++;
        }

        else if (op == LEA) //获取子函数参数地址,存入ax中
        {
            ax = (int)(bp + *pc);
            pc++;
        }


        //以下是运算符指令实现,,栈顶元素与寄存器ax元素比较
        else if (op == OR)
        {
            ax = *sp | ax;
            sp++;
        }
        else if (op == XOR)
        {
            ax = *sp ^ ax;
            sp++;
        }
        else if (op == AND)
        {
            ax = *sp & ax;
            sp++;
        }
        else if (op == EQ)
        {
            ax = (*sp == ax);
            sp++;
        }
        else if (op == NE){
            ax = (*sp != ax);
            sp++;
        }
        else if (op == LT)
        {
            ax = (*sp < ax);
            sp++;
        }
        else if (op == LE){
            ax = (*sp <= ax);
            sp++;
        }
        else if (op == GT){
            ax = (*sp > ax);
            sp++;
        }
        else if (op == GE){
            ax = (*sp >= ax);
            sp++;
        }
        //栈顶元素向左移ax位
        
        else if (op == SHL){
            ax = (*sp << ax);
            sp++;
        }
        //栈顶元素向右移ax位
        else if (op == SHR){
            ax = (*sp >> ax);
            sp++;
        }
        else if (op == ADD){
            ax = (*sp + ax);
            sp++;
        }
        else if (op == SUB){
            ax = (*sp - ax);
            sp++;
        }
        else if (op == MUL){
            ax = (*sp * ax);
            sp++;
        }
        else if (op == DIV){
            ax = (*sp / ax);
            sp++;
        }
        else if (op == MOD){
            ax = (*sp % ax);
            sp++;
        }


        //以下是内置函数exit open close read printf malloc memset memcpy函数实现
        else if (op == EXIT){
            printf("exit(%d)\n",*sp);
            return *sp;
        }
        else if (op == OPEN){
            //将sp[0]作为文件打开方式，sp[1]强转成char *作为文件名的地址，返回文件描述符存入ax中
            //sp[0]是栈顶元素，sp[1]是栈顶下一个元素
            ax = open((char *)sp[1], sp[0]);
        }
        else if (op == CLOS){
            //将栈顶元素作为文件描述符，返回0表示成功，-1表示失败，存入ax中
            //close(文件描述符)
            ax = close(*sp);
        }
        else if (op == READ){
            //将栈顶元素作为读取的长度，sp[1]作为缓冲区指针，sp[2]作为文件描述符，返回读取的字节数存入ax中。返回0：文件已到达末尾；返回-1：读取失败。
            //read(文件描述符，缓冲区指针，读取字节数)
            ax = read(sp[2], (char *)sp[1], *sp);
        }
        else if (op == PRTF){
            tmp = sp + pc[1];
            //printf返回成功输出字符数，存入ax中
            ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
        }
        else if (op == MALC){
            ax = (int)malloc(*sp);  //将malloc分配成功返回的地址作为longlong数据存放在ax中
        }
        else if (op == MSET){
            //将memset返回的目标内存地址作为数据存入ax中
            //memset(目标内存地址，初始化值，初始化长度) 返回 void *
            ax = (int)memset((char *)sp[2], sp[1], *sp);
        }
        else if (op == MCMP){
            //将比较结果作为数据存入ax中，相等返回0， 不等返回非0
            //memecmp(串1，串2，比较长度)
            ax = memcmp((char *)sp[2], (char *)sp[1], *sp);
        }
        else
        {
            printf("unknown instruction: %d\n", op);
            return -1;
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

    old_src = malloc(poolsize); // 分配内存,如果成功返回指针，否则返回NULL
    if (old_src){
        src = old_src;  //将old_src的地址赋值给src，方便后面next()函数使用
    }
    else
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

    old_text = malloc(poolsize);
    if (old_text)
    {
        text = old_text;
    }
    else
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

    //把text、data、stack内容全部置为0
    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    //初始时sp指向栈底，即高地址，bp也指向栈底（现在还不用，栈内也没东西，就指向栈底了）
    bp = sp = (int *)((int)stack + poolsize);
    //ax一般用于存放计算结果，先置为0
    ax = 0;

    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;

    program();
    return eval();
}