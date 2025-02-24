#!/bin/bash
#编译01.c
gcc -g 02.c -o 02 

#如果编译成功，则输出编译成功
if [ $? -eq 0 ]; then
    echo "02.c编译成功"
else
    echo "02.c编译失败"
    exit 1
fi

#检测test.txt是否存在，存在则删除
if [ -f "./output_set/02test.txt" ]; then
    rm ./output_set/02test.txt
fi

#运行02.c并将输出重定向到02test.txt
./02 test.c >> ./output_set/02test.txt


echo "02.c解析test.c完成,生成文件./output_set/02test.txt"