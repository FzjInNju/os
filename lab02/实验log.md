[toc]

# 1. 实验前准备

## 1.1简单地学一下makefile

教程网站https://seisman.github.io/how-to-write-makefile/introduction.html

## 1.2 理解fat

orange书

1. 把软盘做成FAT12格式。FAT12的DOS时代就开始使用的文件系统
2. 所有的文件系统都会把磁盘划分为若干层次

- 扇区Sector：磁盘上的最小数据单元
- 簇Cluster：一个或多个扇区
- 分区Partition：整个文件系统

3. **引导扇区**：0扇区，里面有BPB（BIOS Parameter Block），记录软盘的基本信息

   **两个FAT表**：1-9 10-18扇区，

   **根目录区**：19-   长度非固定

   **数据区**：长度非固定

4. 引导扇区：找到文件，将文件内容放入内存

   ![image-20211122142212484](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211122142212484.png)

5. 根目录区

   **由条目Directory Entry组成**，共**BPB_RootEntCnt**个Entry，每一个Entry **32** 字节![image-20211122142504579](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211122142504579.png)

   先找到**DIR_Name**，再找到**DIR_FstClus**即开始簇号，再获取文件大小**DIR_FileSize**

   计算根目录区占用的扇区数：![image-20211122142844681](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211122142844681.png)数据区开始扇区号 = 根目录区开始扇区号 + 根目录区占用的扇区数

   文件开始扇区号 =数据区开始扇区号 + （条目对应的开始簇号 - 2） //因为数据区的第一个簇号是2

   如果文件大于512字节，则需要 **FAT表** 来获知之后的文件内容所在位置

6. FAT表

   FAT1 和 FAT2 是相同的，FAT表中每12bit为一个**FAT Entry，保存文件的下一个簇号，大于等于0xFF8表示这个簇是文件的最后一个簇**

   **第0个和第1个FAT Entry始终不使用*

   ![image-20211122143918533](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211122143918533.png)

   

   

   

   

# 2. 实验步骤

## 2.1 制作FAT12镜像  ppt3

在linux中选择一个待工作文件夹

terminal

```
mkfs.fat -C a.img 1440  //在当前目录(.)下创建一个新的软盘镜像a.img
mkdir mount             //在当前目录下创建一个新目录(./mount)作为挂载点
sudo mount a.img mount  //将镜像./a.img挂载到./mount下
                        // ☆ 每次都要mount
sudo mkdir 文件夹名
sudo touch 文件名
sudo rm -r 文件夹/文件名
sudo vim 文件名
	i 
	“esc” shift+zz   // 保存并关闭
```

![image-20211110012057332](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211110012057332.png)

## 2.2 汇编/汇编编译 + 链接 的理解和指令   ppt1 ppt2

1. c语言生成可执行文件过程

   | 过程名称 | 生成文件       | 具体过程                           |
   | -------- | -------------- | ---------------------------------- |
   | 预处理   | .c（纯c文件）  | 替换宏                             |
   | 编译     | .s（汇编程序） | 词法，语法，语义分析生成汇编       |
   | 汇编     | .o（目标文件） |                                    |
   | 链接     | 可执行         | 将多个.o文件重定位拼接成可执行文件 |

   

![image-20211110013920267](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211110013920267.png)

2. 实际情况：

```
nasm -f elf -o func.o func.asm //nasm汇编汇编语言得到目标文件.o
gcc -std=c++11 -o hello func.o main.c  //gcc编译C语言，之后链接目标文件得到可执行文件
./hello
```

## 2.3 makefile的编写

#### makefile 基本格式

```
生成文件名：指令所需文件
	指令
生成文件名：指令所需文件
	指令
   ......
.PHONY:clean
clean:
	rm -rf 以上指令生成的所有文件
```

#### ex

```
main: main.cpp my_print.o
	g++ -std=c++11 -m32 main.cpp my_print.o -o main
	./main
my_print.o: my_print.asm
	nasm -f elf32 my_print.asm -o my_print.o
.PHONY:clean
clean:
	rm -rf my_print.o main
```

#### 命令行操作

```
make clean   // 执行rm -rf my_print.o main
make    // 执行以上代码，即生成可执行文件main并执行
```

## 2.3 程序的编写



