#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#define int long long

int token = 0;
char *src = 0;
char *old_src = 0;
int poolsize = 0;
int line = 0;

//存储程序段的起始地址
int *text = 0;
int *old_text = 0;
//栈起始地址
int *stack = 0;
//数据段起始地址
char *data = 0;

// 初始化bp基址寄存器指针、sp指向栈顶元素的指针寄存器、pc程序计数器、ax通用寄存器
int *bp, *sp, *pc;
int ax = 0;
int cycle = 0;

int token_val = 0;
int *current_id = 0;
int *symbols = 0;

// 初始化定义类型，表达式类型
int basetype = 0;
int expr_type = 0;

// 调用子函数的地址基址
int index_of_bp = 0;

// 标识符结构体
struct identifier
{
    int token;
    int hash; // 标识符的哈希值
    char *name;
    int type;  // 标识符的类型，例如int float等
    int class; // 如数字，局部变量，全局变量等
    int value; // 标识符的值,函数则存函数地址
    // Bxxxxx 表示当局部变量与全局变量重名时，保存全局变量的信息，如type class value等
    int Btype;
    int Bclass;
    int Bvalue;
};

// 枚举指令集
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

enum
{
    Num = 128,
    Fun,
    Sys,
    Glo,
    Loc,
    Id,
    Char,
    Else,
    Enum,
    If,
    Int,
    Return,
    Sizeof,
    While,
    Assign,
    Cond,
    Lor,
    Lan,
    Or,
    Xor,
    And,
    Eq,
    Ne,
    Lt,
    Gt,
    Le,
    Ge,
    Shl,
    Shr,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Inc,
    Dec,
    Brak
};

// 将这些枚举值作为数组的索引，方便按名称访问，严格有序！！！！！！！！
enum
{
    Token,
    Hash,
    Name,
    Type,
    Class,
    Value,
    Btype,
    Bclass,
    Bvalue,
    IdSize
};

// char int pointer 三种类型
enum
{
    CHAR,
    INT,
    PTR
};

void match(int tk)
{ // 将next函数包装起来 如果不是预期值，则报错
    if (token == tk)
    {
        next();
    }
    else
    {
        printf("expexted token: %d(%c), got token: %d(%c)\n", tk, tk, token, token);
        exit(-1);
    }
}

void global_declaration()
{
    // global_declaration ::= enum_decl | variable_decl | function_decl
    // enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] } '}'
    // variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
    // function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'
    int type = 0;
    int i = 0;
    basetype = INT;
    // 匹配枚举类型
    if (token == Enum)
    {
        match(Enum);
        if (token != '{')
        {
            match(Id);
        }
        if (token == '{')
        {
            match('{');
            // 枚举声明
            enum_declaration();
            match('}');
        }
        match(';');
        return;
    }
    // 匹配int类型
    if (token == Int)
    {
        match(Int); // 因为basetype已经初始化为INT，所以不用再赋值
    }
    // 匹配char类型
    if (token == Char)
    {
        match(Char);
        basetype = CHAR;
    }

    // 处理变量 / 函数声明
    while (token != ';' || token != '}')
    {
        type = basetype;
        // 匹配指针定义类型 例如 int **abc;
        while (token == '*')
        {
            match(Mul);
            // 一级指针加一个PTR
            type += PTR;
        }
        // 检测是否符合标识符的命名规则
        if (token != Id)
        {
            printf("%d:bad global declaration.\n", line);
            exit(-1);
        }
        // 检测是否重定义
        if (current_id[Class])
        {
            printf("%d:redeclaration of %s.\n", line, current_id[Name]);
            exit(-1);
        }

        // 不管上述是否匹配到，但现在已经有一个合法ID了，先记录下来
        match(Id);
        current_id[Type] = type;

        // 匹配函数定义
        if (token = '(')
        {
            current_id[Class] = Fun;
            //函数地址
            current_id[Value] = (int)text + 1;
            // 函数声明
            function_declaration();
        }
        else
        {
            // 变量声明
            current_id[Class] = Glo;
            current_id[Value] = (int)data;
            data = data + sizeof(int);
        }

        // 继续处理下一个声明
        if (token = ',')
        {
            match(',');
        }
    }
}

void enum_declaration()
{
    int i = 0;
    while (token != '}')
    {
        if (token != Id)
        {
            printf("%d:bad enum identifier:%d.\n", line, token);
            exit(-1);
        }
        next();
        if (token == Assign)
        {
            next();
            if (token != Num)
            {
                printf("%d:bad enum value:%d.\n", line, token);
                exit(-1);
            }
            i = token_val;
            next();
        }

        current_id[Class] = Num;
        current_id[Value] = i;
        current_id[Type] = INT;
        i++;
    }
}

void function_declaration()
{
    // parameter_decl ::= type {'*'} id {',' type {'*'} id | parameter_decl}
    // body_decl ::= {variable_decl}{statement}
    // statement ::= non_empty_statement | empty_statement
    // non_empty_statement ::= if_statement | while_statement | '{' statement '}'| 'return' expression | expression ';'
    // if_statement ::= 'if' '(' expression ')' statement ['else' non_empty_statement]
    // while_statement ::= 'while' '(' expression ')' non_empty_statement
    match('(');
    function_parameter();
    match(')');
    match('{');
    function_body();
    // match('}');       //此处不先匹配 } 是因为在函数global_declaration中 函数循环结束的标志是匹配到;或者}，如果此时消耗，则函数global_declaration可能死循环

    current_id = symbols;
    // 遍历喜欢每一个变量 如果有和全局重名的情况，恢复到全局变量的类型和值（上面函数function_body已经将重名变量的信息改为局部变量的信息，因为在函数体内，局部变量是老大）
    while (current_id[Token])
    {
        if (current_id[Class] == Loc)
        {
            current_id[Class] = current_id[Bclass];
            current_id[Type] = current_id[Btype];
            current_id[Value] = current_id[Bvalue];
        }
        // 下一个ID
        current_id += IdSize;
    }
}

void function_body()
{
    // body_decl ::= {variable_decl}{statement}
    int pos_local = index_of_bp; // 局部变量地址
    int type = 0;

    //识别函数体内局部变量定义
    while (token == Int || token == Char)
    {
        basetype = (token == Int) ? INT : CHAR;
        match(token);
        while(token != ';'){
            type = basetype;
            while(token == Mul){
                match(Mul);
                type += PTR;
            }
            if (token!= Id){
                printf("%d:bad variable declaration.\n", line);
                exit(-1);
            }
            if (current_id[Class] == Loc){
                printf("%d:duplicate variable declaration.\n", line);
                exit(-1);
            }
            match(Id);
    
            current_id[Bclass] = current_id[Class];
            current_id[Class] = Loc;
    
            current_id[Btype] = current_id[Type];
            current_id[Type] = type;
    
            current_id[Bvalue] = current_id[Value];
            pos_local++;
            //存下变量的地址，因为此时变量还没赋值
            current_id[Value] = pos_local;
    
            if(token == ','){
                match(',');
            }
    
        }
        match(';');
    }
    
    text++;
    *text = ENT;
    text++;
    *text = pos_local - index_of_bp;    //如果函数体内没有定义局部变量，pos_local - index_of_bp = 0

    while(token!= '}'){
        statement();
    }
    text++;
    *text = LEV;

}
void function_parameter()
{
    // parameter_decl ::= type {'*'} id {',' type {'*'} id | parameter_decl}
    int type = 0;
    int params = 0;
    while (token != ')')
    {
        type = INT;
        if (token == Int)
        {
            match(Int);
        }
        else if (token == Char)
        {
            match(Char);
            type = CHAR;
        }

        if (token == Mul)
        {
            match(Mul);
            type += PTR;
        }

        if (token != Id)
        {
            printf("%d:bad parameter declaration.\n", line);
            exit(-1);
        }

        if (current_id[Class] == Loc)
        {
            printf("%d: duplicate parameter declaration\n", line);
            exit(-1);
        }

        match(Id);

        // 全局变量信息存入B
        current_id[Bclass] = current_id[Class];
        // 标识为局部变量
        current_id[Class] = Loc;

        current_id[Btype] = current_id[Type];
        current_id[Type] = type;

        current_id[Bvalue] = current_id[Value];
        current_id[Value] = params;
        params++;

        if (token == ',')
        {
            match(',');
        }
    }
    // bp向下为子函数局部变量地址
    index_of_bp = params + 1;
}

// read the point of src and store the point in token as an integer.
void next()
{
    char *last_pos = 0;
    int hash = 0;
    while (token = *src)
    {
        ++src;
        if (token == '\n')
        {
            ++line;
        }
        // 宏定义直接跳过，因为不支持
        else if (token == '#')
        {
            while (*src != 0 && *src != '\n')
            {
                ++src;
            }
        }
        // 识别标识符
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || token == '_')
        {
            last_pos = src - 1;
            hash = token;

            // 计算变量的哈希值
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || *src == '_')
            {
                hash = hash * 147 + *src;
                src++;
            }

            // 读完一个标识符后，在标识符表中线性查找,即查symbols数组
            current_id = symbols;
            while (current_id[Token])
            {
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos))
                {
                    token = current_id[Token];
                    return;
                }
                current_id += IdSize;
            }

            // 存新ID
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        // 识别数字，有三种：十进制、十六进制（以0x开头）、八进制（以0开头）
        else if (token >= '0' && token <= '9')
        {
            token_val = token - '0';
            // 十进制
            if (token_val > 0)
            {
                while ((*src >= '0' && *src <= '9'))
                {
                    token_val = token_val * 10 + *src - '0';
                    src++;
                }
            }
            else if (token_val == 0)
            {
                // 十六进制
                if (*src == 'x' || *src == 'X')
                {
                    src++;
                    token = *src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F'))
                    {
                        // 十六进制转十进制
                        //  token_val = token_val * 16 + (token >= '0' && token <= '9'? token - '0' : token - 'A' + 10);
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                    }
                }
                else if (*src >= '0' && *src <= '7')
                {
                    // 八进制
                    while ((*src >= '0' && *src <= '7'))
                    {
                        token_val = token_val * 8 + *src - '0';
                        src++;
                    }
                }
                else
                {
                    printf("error: unknow number type.\n only support dec(ex.123) hex(ex.0x123) and oct(ex.0123).");
                }
            }
            else
            {
                printf("error: unknow number type.\n only support dec(ex.123) hex(ex.0x123) and oct(ex.0123).");
            }
            token = Num;
            return;
        }
        // 识别字符串
        else if (token == '"' || token == '\'')
        {
            last_pos = data;
            while (*src != token && *src != 0)
            {
                token_val = *src;
                src++;
                // 遇到转义字符
                if (token_val == '\\')
                {
                    token_val = *src;
                    src++;
                    // 特殊的转义字符
                    if (token_val == 'n')
                    {
                        token_val = '\n';
                    }
                    else if (token_val == 't')
                    {
                        token_val = '\t';
                    }
                    else if (token_val == 'r')
                    {
                        token_val = '\r';
                    }
                    else if (token_val == 'b')
                    {
                        token_val = '\b';
                    }
                    else if (token_val == 'f')
                    {
                        token_val = '\f';
                    }
                    else if (token_val == 'v')
                    {
                        token_val = '\v';
                    }
                    else
                    {
                        printf("error: unknow escape character type.\n only support \\n \\t \\r \\b \\f and \\v");
                    }
                }
                // 字符串结束，逐个字符压入data中
                if (token == '"')
                {
                    *data = token_val;
                    data++;
                }
            }
            src++;
            if (token == '"')
            {
                token_val = (int)last_pos;
            }
            else
            {
                // 单字符以对应数字返回
                token = Num;
            }
            return;
        }
        // 识别注释，不支持/**/类注释，只支持逐行//类注释
        else if (token == '/')
        {
            if (*src == '/')
            {
                // 跳过这一行
                while (*src != 0 && *src != '\n')
                {
                    src++;
                }
            }

            else
            {
                token = Div;
                return;
            }
        }
        // 识别运算符
        else if (token == '=')
        {
            if (*src == '=')
            {
                token = Eq;
            }
            else
            {
                token = Assign;
            }
            src++;
            return;
        }
        else if (token == '+')
        {
            if (*src == '+')
            {
                token = Inc;
            }
            else
            {
                token = Add;
            }
            src++;
            return;
        }
        else if (token == '-')
        {
            if (*src == '-')
            {
                token = Dec;
            }
            else
            {
                token = Sub;
            }
            src++;
            return;
        }
        else if (token == '!')
        {
            if (*src == '=')
            {
                token = Ne;
            }
            src++;
            return;
        }
        else if (token == '<')
        {
            if (*src == '=')
            {
                token = Le;
            }
            else if (*src == '<')
            {
                token = Shl;
            }
            else
            {
                token = Lt;
            }
            src++;
            return;
        }
        else if (token == '>')
        {
            if (*src == '=')
            {
                token = Ge;
            }
            else if (*src == '>')
            {
                token = Shr;
            }
            else
            {
                token = Gt;
            }
            src++;
            return;
        }
        else if (token == '|')
        {
            if (*src == '|')
            {
                token = Lor;
            }
            else
            {
                token = Or;
            }
            src++;
            return;
        }
        else if (token == '&')
        {
            if (*src == '&')
            {
                token = Lan;
            }
            else
            {
                token = And;
            }
            src++;
            return;
        }
        else if (token == '^')
        {
            token = Xor;
            return;
        }
        else if (token == '*')
        {
            token = Mul;
            return;
        }
        else if (token == '%')
        {
            token = Mod;
            return;
        }
        else if (token == '[')
        {
            token = Brak;
            return;
        }
        else if (token == '?')
        {
            token = Cond;
            return;
        }
        // 遇到以下字符直接返回，但是为什么左中括号和右中括号分开了？？数组的原因？？ 对 就是因为数组的原因
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ',' || token == ']' || token == ':')
        {
            return;
        }
    }
    return;
}

void expression(int level)
{
    // do nothing
}

// 循环读取文件，直到token小于0（因为token是ASCII，所以没有小于0 的情况，等于0就是结束符）
void program()
{
    next(); // get next token
    // while (token > 0)    //这样不能读取中文，因为中文的ASCII码大于0，转换成token的时候可能为负值
    while (token != 0) // 改为token!=0
    {
        global_declaration();
    }
}

// 虚拟机接口
int eval()
{
    int op, *tmp;
    while (1)
    {
        op = *pc++;    // 初始化op为当前指令
        if (op == IMM) // 将立即数载入ax
        {
            ax = *pc++;
        }
        else if (op == LC) // 将对应地址中的字符载入ax中，要求ax中存放地址
        {
            ax = *(char *)ax;
        }
        else if (op == LI) // 将对应地址中的整数载入ax中，要求ax中存放地址
        {
            ax = *(int *)ax;
        }
        else if (op == SC) // 将ax中的数据作为字符存入对应地址中，要求栈顶存放地址
        {
            ax = *(char *)*sp++ = ax; // 原写法
            // 新写法
            //  *sp = ax;
            //  ax = *(char *)*sp;
            //  sp++;
        }
        else if (op == SI) // 将ax中的数据作为整数存入对应地址中，要求栈顶存放地址
        {
            *(int *)*sp++ = ax; // 原写法
            // 新写法
            //  *sp = ax;
            //  ax = *(int *)*sp;
            //  sp++;
        }
        else if (op == PUSH) // 将ax中的数据压入栈中
        {
            // *--sp = ax;  //原写法，太炫了，改成下面的写法
            sp--;
            *sp = ax;
        }
        else if (op == JMP)
        {
            pc = (int *)*pc;
        }
        else if (op == JZ) // 如果ax为0，跳转到对应地址，否则继续执行下一条指令
        {
            pc = ax ? pc + 1 : (int *)*pc;
        }
        else if (op == JNZ) // 如果ax不为0，跳转到对应地址，否则继续执行下一条指令
        {
            pc = ax ? (int *)*pc : pc + 1;
        }
        else if (op == CALL) // 把现在在执行的指令的下一条压栈，然后转到子函数执行
        {
            sp--;
            *sp = (int)(pc + 1);
            pc = (int *)*pc;
        }
        else if (op == LEV)
        {
            // 将bp指向 原来指向的内容 所指向的内容，然后将这块空间出栈
            sp = bp;
            bp = (int *)*sp;
            sp++;

            // 从子函数返回
            pc = (int *)*sp;
            sp++;
        }
        else if (op == ENT)
        {
            sp--;
            *sp = (int)bp; // 把bp这个地址作为数据压栈，即保留bp数据
            bp = sp;
            sp = sp - *pc; // 在栈上保留*pc这么多空间
            pc++;          // pc指向下一条指令
        }
        else if (op == ADJ) // 将调用子函数时压栈的数据清除
        {
            // 回收*pc这么多空间，然后pc指向下一条指令
            sp = sp + *pc;
            pc++;
        }

        else if (op == LEA) // 获取子函数参数地址,存入ax中
        {
            ax = (int)(bp + *pc);
            pc++;
        }

        // 以下是运算符指令实现,,栈顶元素与寄存器ax元素比较
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
        else if (op == NE)
        {
            ax = (*sp != ax);
            sp++;
        }
        else if (op == LT)
        {
            ax = (*sp < ax);
            sp++;
        }
        else if (op == LE)
        {
            ax = (*sp <= ax);
            sp++;
        }
        else if (op == GT)
        {
            ax = (*sp > ax);
            sp++;
        }
        else if (op == GE)
        {
            ax = (*sp >= ax);
            sp++;
        }
        // 栈顶元素向左移ax位

        else if (op == SHL)
        {
            ax = (*sp << ax);
            sp++;
        }
        // 栈顶元素向右移ax位
        else if (op == SHR)
        {
            ax = (*sp >> ax);
            sp++;
        }
        else if (op == ADD)
        {
            ax = (*sp + ax);
            sp++;
        }
        else if (op == SUB)
        {
            ax = (*sp - ax);
            sp++;
        }
        else if (op == MUL)
        {
            ax = (*sp * ax);
            sp++;
        }
        else if (op == DIV)
        {
            ax = (*sp / ax);
            sp++;
        }
        else if (op == MOD)
        {
            ax = (*sp % ax);
            sp++;
        }

        // 以下是内置函数exit open close read printf malloc memset memcpy函数实现
        else if (op == EXIT)
        {
            printf("exit(%d)\n", *sp);
            return *sp;
        }
        else if (op == OPEN)
        {
            // 将sp[0]作为文件打开方式，sp[1]强转成char *作为文件名的地址，返回文件描述符存入ax中
            // sp[0]是栈顶元素，sp[1]是栈顶下一个元素
            ax = open((char *)sp[1], sp[0]);
        }
        else if (op == CLOS)
        {
            // 将栈顶元素作为文件描述符，返回0表示成功，-1表示失败，存入ax中
            // close(文件描述符)
            ax = close(*sp);
        }
        else if (op == READ)
        {
            // 将栈顶元素作为读取的长度，sp[1]作为缓冲区指针，sp[2]作为文件描述符，返回读取的字节数存入ax中。返回0：文件已到达末尾；返回-1：读取失败。
            // read(文件描述符，缓冲区指针，读取字节数)
            ax = read(sp[2], (char *)sp[1], *sp);
        }
        else if (op == PRTF)
        {
            tmp = sp + pc[1];
            // printf返回成功输出字符数，存入ax中
            ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
        }
        else if (op == MALC)
        {
            ax = (int)malloc(*sp); // 将malloc分配成功返回的地址作为longlong数据存放在ax中
        }
        else if (op == MSET)
        {
            // 将memset返回的目标内存地址作为数据存入ax中
            // memset(目标内存地址，初始化值，初始化长度) 返回 void *
            ax = (int)memset((char *)sp[2], sp[1], *sp);
        }
        else if (op == MCMP)
        {
            // 将比较结果作为数据存入ax中，相等返回0， 不等返回非0
            // memecmp(串1，串2，比较长度)
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
    int i = 0;
    int fd = 0;
    int *idmain = 0;

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
    if (old_src)
    {
        src = old_src; // 将old_src的地址赋值给src，方便后面next()函数使用
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
    if (!(symbols = malloc(poolsize)))
    {
        printf("could not malloc(%d) for symbols table area.\n");
        return -1;
    }

    // 把text、data、stack,symbols内容全部置为0
    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);
    memset(symbols, 0, poolsize);

    // 初始时sp指向栈底，即高地址，bp也指向栈底（现在还不用，栈内也没东西，就指向栈底了）
    bp = sp = (int *)((int)stack + poolsize);
    // ax一般用于存放计算结果，先置为0
    ax = 0;

    i = Char;
    src = "char else enum if int return sizeof while"
          "open read close printf malloc memset memcmp exit void main";

    while (i <= While)
    {
        next();
        current_id[Token] = i;
        i++;
    }
    i = OPEN;
    while (i <= EXIT)
    {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i;
        i++;
    }
    next();
    current_id[Token] = Char;
    next();
    idmain = current_id;

    program();
    return eval();
}