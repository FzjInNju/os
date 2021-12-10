touch hello.asm

gedit hello.asm

在文件中输入汇编代码，保存后退出

编译：nasm -f elf32 hello.asm

链接：ld -s -o hello hello.o

运行：./hello

