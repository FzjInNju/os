[toc]



# Linux汇编语言开发

## 一、简介

汇编是底层语言，功能强大，能最大限度发挥硬件的性能

不受编译器限制，对生成的二进制代码完全控制

## 二、Linux汇编语法格式

1. push eax

2. push 1

   1是立即操作数

3. add eax，1

4. mov al，byte ptr val

   操作数字长用“byte ptr“和”word ptr“前缀来表示

5. jmp far section:offset 远程转移指令

   call far section:offset 远程子调用指令

   ret far stack_adjust 远程返回指令

6. Linux工作在保护模式下，用的是32位线性地址，不需要考虑段地址*16加上偏移量，采用如下的地址计算方式：

   `disp + base + index * scale`

## 三、helloworld

![image-20211018185922739](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211018185922739.png)

## 四、linux汇编工具

汇编器、连接器、调试器

1. 汇编器 NASM

   汇编语言源程序 -> 二进制形式目标代码

   ```[xiaowp@gary code]$ nasm -f elf -F stabs hello -o hello.o```

   • -F stabs 告知汇编器向输出⽂件添加调试信息

   ​    stabs 是调试信息的格式 (可用nasm -felf -y查看) 

   ​    gdb和ddd可以利⽤其中的调试信息

   • nasm查看⽀持的输出格式，使⽤ nasm -hf (elf elf_i386都是输出格式)

2. 连接器 ld

   可以将多个目标代码连接成一个可执行代码（没规定一定是多个，只是为了方便分成几个模块单独开发）

   `[xiaowp@gary code]$ ld -s -o hello hello.o`

   或者 `ld -m elf_i386 hello.o -o hello`

   （汇编语言和c嵌套编程时可用gcc连接，命令为 gcc -o hello_stabs hello_stabs.o）

3. 运行

   在hello可执行文件所在文件夹打开终端，输入 ./hello

4. 调试器ALD或GDB

   设置断点中断程序运行、查看变量和寄存器的当前值

   ALD:

   ```ald
   ald hello(可执行文件名)
   disassemble -s .text ;对代码段进行反汇编，得到代码所在的内存地址
   break 0x08048088 ;设置断点
   run ;执行程序到断点处
   next ;单步调试
   help ;获得调试命令的详情列表
   ```

   gdb：

   • gdb hello_stabs (gnu debugger)

   • ddd hello_stabs (gdb’s UI）

## 五、系统调用

程序需要调用操作系统所提供的服务，例如输入、输出、退出等，这就是系统调用

![image-20211018185922739](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211018185922739.png)

1. `int 0x80`执行时，开始系统调用

2. eax存系统调用号（即决定调用哪种函数）：

   可在文件 /usr/include/bits/syscall.h 或者 https://chromium.googlesource.com/chromiumos/docs/+/HEAD/constants/syscalls.md 中找到

3. 参数依次存在：ebx、ecx、edx、esi、edi五个寄存器中

   参数也可以存在：一段连续的内存中，此时在ebx中保存指向该内存区域的指针

4. eax存返回值

## 六、命令行参数

在 Linux 操作系统中，当一个可执行程序通过命令行启动时，其所需的参数将被保

存到栈中：首先是 argc，然后是指向各个命令行参数的指针数组 argv，最后是指向

环境变量的指针数据 envp。

## 七、GCC内联汇编

第一个作业用不到c，暂时不看了，以后再看

需要解决的问题：分配寄存器、和c代码结合

```c
__asm__("push eax \\n\\t"
        "mov eax,0 \\n\\t");

//完整的内联汇编格式
__asm__("asm statements" : outputs : inputs : registers-modified);
```

