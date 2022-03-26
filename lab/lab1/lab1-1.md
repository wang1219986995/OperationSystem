lab1 官方地址： https://pdos.csail.mit.edu/6.828/2018/labs/lab1/

## Part 1: PC Bootstrap

#### Exercise 1. 

> 熟悉汇编语言，了解AT&T 汇编语法（注意与Intel汇编语法的区别）

1. 汇编语言笔记

   **TODO：占坑，准备笔记**

2. 6.828 中的xv6代码的汇编语言为 AT&T格式，与Intel格式的汇编有所区别，应注意以下区分，防止因以下区别导致的错误理解。

> AT&T 与 Intel 格式相比应注意的地方
>
> 参考链接 http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html
>
> 1、寄存器引用：
>
> ```shell
> AT&T:  %eax
> Intel: eax
> ```
>
> 2、操作数顺序 ：
>
> ```shell
> AT&T:  movl %eax(source), %ebx(destination)
> Intel: mov ebx(destination), eax(source)
> ```
>
> 3、常数/立即数格式：
>
> 常数：
>
> ```
> AT&T:  movl $_booga, %eax
> Intel: mov eax, _booga
> ```
>
> 立即数：
>
> ```
> AT&T:  movl $0xd00d, %ebx
> Intel: mov ebx, d00dh
> ```
>
> 4、操作数长度:
>
> You must suffix the instruction with one of b, w, or l to specify the width of the destination register as a byte, word or longword. If you omit this, GAS (GNU assembler) will attempt to guess. You don't want GAS to guess, and guess wrong! Don't forget it.
>
> ```
> AT&T:  movw %ax, %bx
> Intel: mov bx, ax
> ```
>
> The equivalent forms for Intel is byte ptr, word ptr, and dword ptr, but that is for when you are... 
>
> **TODO：占坑，未完待续**

> 5、绝对跳转 （jmp/call）的操作数前加上 “*”作前缀
>
> 6、远跳转指令和远调用指令的操作码为：ljump, lcall 而 Intel 格式的是 jmp far, call far
>
> 7、远程返回指令 lret $stack_adjust 而 Intel 格式是 ret far stack_adjust
>
> 8、寻址方式 表示地址：base + index*scale + disp
>
> 　　　　　　at&t　　section:disp(base,index,scale)
>
> 　　　　　　Intel　　section:[base + index*scale + disp]
>
> 9、嵌入汇编 
>
> 



#### Exercise 2. 

> 使用GDB的 si 指令单步执行，查看BIOS阶段在做什么。



[f000:fff0]    0xffff0: ljmp   $0xf000,$0xe05b

[f000:e05b]    0xfe05b:cmpl   $0x0,%cs:0x6ac8

[f000:e062]    0xfe062:jne    0xfd2e1

[f000:e066]    0xfe066:xor    %dx,%dx

[f000:e068]    0xfe068:mov    %dx,%ss

[f000:e06a]    0xfe06a:mov    $0x7000,%esp

[f000:e070]    0xfe070:mov    $0xf34c2,%edx

[f000:e076]    0xfe076:jmp    0xfd15c

[f000:d15c]    0xfd15c:mov    %eax,%ecx

[f000:d15f]    0xfd15f:cli    

[f000:d160]    0xfd160:cld   

[f000:d161]    0xfd161:mov    $0x8f,%eax

[f000:d167]    0xfd167:out    %al,$0x70

[f000:d169]    0xfd169:in     $0x71,%al

[f000:d16b]    0xfd16b:in     $0x92,%al

[f000:d16d]    0xfd16d:or     $0x2,%al

[f000:d16f]    0xfd16f:out    %al,$0x92

[f000:d171]    0xfd171:lidtw  %cs:0x6ab8

[f000:d177]    0xfd177:lgdtw  %cs:0x6a74

[f000:d17d]    0xfd17d:mov    %cr0,%eax

[f000:d180]    0xfd180:or     $0x1,%eax

[f000:d184]    0xfd184:mov    %eax,%cr0

[f000:d187]    0xfd187:ljmpl  $0x8, \$0xfd18f



### Part 2: The Boot Loader

Boot Loader 的两个作用

1. 从实模式转到保护模式的32位寻址方式；
2. 将内核从磁盘读取到内存中。

#### Exercise 3.

> 在引导扇区的加载地址0x7c00处设置断点，从该地址处跟踪boot/boot.S中命令的执行（可结合obj/boot/boot.asm中的反汇编命令）。使用 `x/i addr` GDB指令反汇编出的代码与boot.asm 中的指令进行比较。
>
> 跟踪进入 boot/main.c 中的 bootmain() 函数中，随后进入 readsect() 函数中。确认readsect() 中的语句与其对应的汇编指令。跟踪至readsect() 的剩余部分，确定`for` 循环的开始和结束。 找出循环结束后要执行的代码，然后逐步执行完引导加载程序的剩余部分。

**Questions：**

- 在什么地方，处理器开始执行32位的代码，什么条件导致了从16位到32位的转换？
- 引导加载程序最后执行的指令和内核的第一条指令分别是什么？
- 内核的第一条指令在哪里？
- 引导加载程序如何决定从磁盘中读取多少个扇区以获取整个内核，该信息是从何处获取到的？



#### Exercise 4.





















































