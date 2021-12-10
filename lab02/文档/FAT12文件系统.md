# FAT12文件系统

## 简介

**file allocation table 文件配置表，记录文件所在位置的表格**

存储介质上组织文件数据的方法

DOS时代的早期**文件系统**

结构非常简单，一直沿用于**软盘**（磁盘的一种，可移动，像u盘，一般存储文件）

基本组织单位

​      字节（Byte）：基本数据单位

​      扇区（Sector）：磁盘中的最小数据单元

​      簇（Cluster）：一个或多个扇区

![image-20211103195807877](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211103195807877.png)![image-20211103195912597](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211103195912597.png)

## FAT12引导扇区（引导程序、主引导区）

主引导区存储的比较重要的信息是文件系统的类型，文件系统逻辑扇区总数，

​      每簇包含的扇区数等。主引导区最后以0x55AA两个字节作为结束，共占用一个扇区。

![image-20211103200036729](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211103200036729.png)![image-20211103200118246](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211103200118246.png)

## FAT表（FAT1 FAT2）

1. FAT表概念

- FAT1、FAT2相互**备份**，内容完全一致
- FAT表是一个**关系图**，记录了文件数据的先后关系
- 每一个FAT表项占用**12 bit**
- FAT表的前2个表项规定不使用

2. FAT表结构

  - 以簇（扇区）为单位存储文件数据(这里一簇等于一扇区大小)

  - FAT表的每个表项（vec[i]）表示文件数据的实际位置（簇）

    （1）DIR_FstClus表示文件第0簇（扇区）的位置

    （2）vec[DIR_FstClus]表示文件第1簇（扇区）的位置

    （3）vec[vec[DIR_FstClus]]表示文件第2簇（扇区）的位置...

![image-20211103201356386](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211103201356386.png)![image-20211103201411294](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211103201411294.png)

## 根目录区

![image-20211103200205439](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211103200205439.png)![image-20211103200219621](C:\Users\junlines\AppData\Roaming\Typora\typora-user-images\image-20211103200219621.png)

```

struct RootEntry
{
    char DIR_Name[11];
    uchar DIR_Attr;
    uchar reserve[10];
    ushort DIR_WrtTime;
    ushort DIR_WrtDate;
    ushort DIR_FstClus;
    uint DIR_FileSize;
};
————————————————
版权声明：本文为CSDN博主「洋葱汪」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/qq_39654127/article/details/88429461
```

