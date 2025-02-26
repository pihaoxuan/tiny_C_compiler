#include <stdio.h>
#include <stdlib.h>

enum{NUM};
int token = 0;
int token_val = 0;
char *line = 0;
char *src = 0;

void next(){
    //跳过空格
    while(*src == ' ' || *src == '\t'){
        src++;
    }
    token = *src;
    src++;
    if(token >= '0' && token <= '9'){
        token_val = token - '0';
        token = NUM;
        while(*src >= '0' && *src <= '9'){
            token_val = token_val * 10 + *src - '0';
            src++;
        }
        return;
    }
}

void match(int tk){
    if(token != tk){
        if(token == 10){
            printf("expexted token: %d(%c), got token: %d(\\n)\n",tk,tk,token);
        }
        else{
            printf("expexted token: %d(%c), got token: %d(%c)\n",tk,tk,token,token);
        }
        exit(-1);
    }
    next();
}

int expr();

//处理括号表达式【本来是可以处理一系列终结符 但是next函数里面实现了处理数字的终结符功能】
int factor(){
    int value = 0;
    if(token == '('){
        match('(');
        value = expr();
        match(')');
    }
    else{
        value = token_val;
        match(NUM);
    }
    return value;
}

//term_tail处理高优先级运算符，比如乘除
int term_tail(int lvalue){
    if(token == '*'){
        match('*');
        int value = lvalue * factor();
        return term_tail(value);
    }
    else if(token == '/'){
        match('/');
        int value = lvalue / factor();
        return term_tail(value);
    }
    else{
        return lvalue;
    }
}

int term(){
    int lvalue = factor();
    return term_tail(lvalue);
}

//expr_tail 处理低优先级运算符 比如加减
int expr_tail(int lvalue){
    if(token == '+'){
        match('+');
        int value = lvalue + term();
        return expr_tail(value);
    }
    else if(token == '-'){
        match('-');
        int value = lvalue - term();
        return expr_tail(value);
    }
    else{
        return lvalue;
    }
}

//后处理低优先级运算 所以value为term函数的返回值
int expr(){
    int value = term();
    return expr_tail(value);
}

int main(int argc, char** argv){
    size_t linecap = 0;
    __ssize_t linelen = 0;
    while(linelen = getline(&line,&linecap,stdin) > 0){
        src = line;
        next();
        printf("%d\n",expr());
    }
    return 0;
}