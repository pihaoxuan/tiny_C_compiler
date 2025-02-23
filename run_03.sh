#!/bin/bash
#编译01.c
gcc -g 03.c -o 03 

#如果编译成功，则输出编译成功
if [ $? -eq 0 ]; then
    echo "03.c编译成功"
else
    echo "03.c编译失败"
    exit 1
fi

#检测test.txt是否存在，存在则删除
if [ -f "03test.txt" ]; then
    rm 03test.txt
fi

#运行03.c并将输出重定向到03test.txt
./03 test.c >> 03test.txt


echo "03.c解析test.c完成,生成文件03test.txt"