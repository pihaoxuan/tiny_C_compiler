#!/bin/bash
#编译01.c
gcc -g 04.c -o 04 

#如果编译成功，则输出编译成功
if [ $? -eq 0 ]; then
    echo "04.c编译成功"
else
    echo "04.c编译失败"
    exit 1
fi

#检测test.txt是否存在，存在则删除
if [ -f "./output_set/04test.txt" ]; then
    rm ./output_set/04test.txt
fi

#运行04.c并将输出重定向到04test.txt
./04 test.c >> ./output_set/04test.txt


echo "04.c解析test.c完成,生成文件04test.txt"