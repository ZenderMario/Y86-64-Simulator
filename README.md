# Y86-64-Simulator

## 模拟器指令介绍

该文档用于帮助您获得该模拟器的大体知识，具体细节大致符合 CSAPP 第四章单周期 CPU 所描述。

下面为该模拟器支持的一些指令：

| Instruction      | Definition                                        |
| ---------------- | ------------------------------------------------- |
| hlt              | stop the processor(not implement yet)             |
| nop              | just increase PC by 1                             |
| rrmov  Ra, Rb    | register-to-register move( include condition move |
| irmov  $i, Rb    | immediate-to-register move                        |
| rmmov  Ra, D(Rb) | register-to-memory move                           |
| mrmov  D(Rb), Ra | memory-to-register move                           |
| OP     Ra, Rb    | integer operation                                 |
| jxx    .dst      | condition jump  ( dst refers to an address        |
| call   .dst      | function call   ( dst refers to an address        |
| ret              | function return                                   |
| push   Ra        | push register value to stack                      |
| pop    Rb        | pop stack value to register                       |
| PRINT  Ra        | print information of registers( for debug         |

下面列出了共计16个寄存器（0 寄存器并不使用以简化代码）。

| Code | Refer to |
| ---- | -------- |
| 0    | %rax     |
| 1    | %rbx     |
| 2    | %rcx     |
| 3    | %rdx     |
| 4    | %rsp     |
| 5    | %rbp     |
| 6    | %rsi     |
| 7    | %rdi     |
| 8    | %r8      |
| 9    | %r9      |
| 10   | %r10     |
| 11   | %r11     |
| 12   | %r12     |
| 13   | %r13     |
| 14   | %r14     |
| 15   | %r15     |
| 16   | NONE     |

关于寄存器寻址方面，该模拟器采用了 imm( R)的寻址方式来确定内存位置。该模拟器使用了 2^16 字节的可寻址内存。

## 如何使用

在可执行文件同目录中创建一个包含指令（具体内容见下），该模拟器从该文件中逐行读取。仓库中自带一个 code.txt，其中已经包含了一个处理汉诺塔的递归问题。

使用 `` g++ .\Unit.cpp .\Reader.cpp .\SEQProcessor.cpp .\main.cpp -o Y86``编译为可执行文件 Y86，运行即可。

执行 Code.txt 指令可得到如下结果（截取部分）

<img src="Pic\1.png" width = "60%" />

上面两个字节输出是 PC寄存器中内容，八个字节的输出是某个寄存器的内容。Code.txt 功能是计算 3 层汉诺塔问题，且只打印了 %r9 寄存器（每次移动的结果），最低三位的含义分别是 A、B 和 C 柱, 两柱为 1 即移动盘子方向，结果正确。

## 读取指令

下面介绍一下该文件具体内容。下面两条指令必须提供在文件首部，设置栈指针为 0x100，即栈从此处开始增长，可以更改栈初始地址。对于 PRINT 指令有 ALL 和 OFF 两种选项，作用是 开/关 寄存器内容显示（每条指令后进行输出）。

```
Set Stack = 0x100
PRINT=ALL
```

声明函数的形式，将 MAIN 入口设置在 0x0 处，PC 总是在 0x0 处执行。函数总是需要显式设置入口地址,如

```
#MAIN = 0x0
#Func = 0xa00
```

除了函数，该模拟器还是用了标签（label）实现条件等功能。如下定义了一个 L1 的标签。

```
.L1:
```

使用 jmp .L1 便可以跳转到改标签对应地址。注意标签地址是自动计算的（根据指令长度）。

初次之外，鉴于 PRINT 指令显示所有寄存器，该模拟器还提供了一条特殊指令 PRINT，使用方式如下，

```
PRINT %rsp
```

在执行该条指令后便直接向标准输出打印 %rsp 寄存器的值。

## 缺点和计划

由于本人的知识和经验不足，该项目比较臃肿（没有使用模式匹配），实现的功能也比较少。注意，该模拟器并没有实现保护的机制，自定义的函数入口可以设置在已有指令的位置，以及数据和指令内存共用的情况。计划可以在之后的版本中解决这些问题。
