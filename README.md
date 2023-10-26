# Y86-64-Simulator
看关于Y86-64模拟器心血来潮做了一发， 并行模拟器要等等（ ![1578BB93](https://user-images.githubusercontent.com/111335804/235477839-623951b2-4ddc-418b-8ec5-b257a63aca26.png)
）

# Tutorial on My ASM

**This page is going to tell you about how to us this ASM**
It's of great significance to understand tasks it could deal with. And we say it first, we dont't
implement float operation like xmm or ymm vector registers to simply this simulator.

Firstly, here is all instructions it supports

| Instruction      | Definition                                        |
|------------------|---------------------------------------------------|
| hlt              | stop the processor                                |
| nop              | just increase PC by 1                             |
| rrmov  Ra, Rb    | register-to-register move( include condition move |
| irmov  $i, Rb    | immediate-to-register move                        |
| rmmov  Ra, D(Rb) | register-to-memory move                           |
| mrmov  D(Rb), Ra | memory-to-register move                           |
| OP     Ra, Rb    | integer operation                                 |
| jxx    .dst      | condition jump  ( dst refers to an address        |
| call   .dst      | function call   ( dst refers to an address                                  |
| ret              | function return                                   |
| push   Ra        | push register value to stack                      |
| pop    Rb        | pop stack value to register                       |
| PRINT  Ra        | print information of registers( for debug         |

Then, we list all registers( 15 in total, 1 marked as none to simplify process)
Those are visible to programmers, you can open an option what we'll talk later
to force all information to be printed out.

| Code | Refer to |
|------|----------|
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

Thirdly, we declare some details. The first is rmmov or mrmov. We choose D( Rb) to locate
memory units, which is base-pointer + bias. 

Last, we talk about what we do in out code.txt( our program read instructions from it);

First Line we always set stacl position : 
```

Set Stack = 0x100
// set the top of the stack to 0x100, because we have 2^16 bytes
// in memory, don't make the top over it

```

Then, we set the entrance of the program is always 0x0( PC starts at 0x0)
You have to declare the address explicitly as 
```
#MAIN = 0x0
...
```
Then you can write various instructions.

Besides, if you want to declare some functions, you have to write its address explicitly.
```
#Func = 0xa00
```

Func is a function name, the declaration above is to set its entrance to 0xa00
.You can declare function anywhere you want except it doesn't destory the memory other function or instrction use.

Last, if you want to use some Label( used in jxx), you can just write it down in text, like
```
.L1:

```

we can calculate its address by context. PRINT(Ra) is what we implement as debug operation to show
register value to detect your code's correctness or our lack of thought. Or you can
declare as later to make every register and condition code information visible.

```
Set Stack = ...
PRINT=ALL

or 

PRINT=OFF

...

PRINT %rsp
```

***Remember, we always choose Rb as the first operand, like sub %rax, %rcx
we always get the result of %rcx - %rax***
