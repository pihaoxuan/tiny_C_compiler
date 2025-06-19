#include <stdio.h>
int main()
{
    // hel你好
    int a, b, c, d;
    a = 1;
    b = a++;

    d = 1;
    c = ++d;
    printf("b:%d\nc:%d\n", b, d);
    return 0;
}