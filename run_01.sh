#!/bin/bash
#编译01.c
gcc -g 01.c -o 01 

#如果编译成功，则输出编译成功
if [ $? -eq 0 ]; then
    echo "01.c编译成功"
else
    echo "01.c编译失败"
    exit 1
fi

#检测test.txt是否存在，存在则删除
if [ -f "01test.txt" ]; then
    rm 01test.txt
fi

#运行01.c并将输出重定向到test.txt
./01 test.c >> ./output_set/01test.txt


#echo "01.c解析完成"