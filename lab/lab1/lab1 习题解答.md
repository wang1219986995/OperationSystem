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

**readsect() 函数与汇编指令对比分析：**

```c
readseg(uint32_t pa, uint32_t count, uint32_t offset)
{
	uint32_t end_pa;
	end_pa = pa + count;
	pa &= ~(SECTSIZE - 1);
	offset = (offset / SECTSIZE) + 1;
	while (pa < end_pa) {
		readsect((uint8_t*) pa, offset);
		pa += SECTSIZE;
		offset++;
	}
}
```



```assembly
00007cdc <readseg>:
{
    7cdc:	55                   	push   %ebp           #调用函数的基址入栈
    7cdd:	89 e5                	mov    %esp,%ebp      
    7cdf:	57                   	push   %edi
    7ce0:	56                   	push   %esi
	offset = (offset / SECTSIZE) + 1;
    7ce1:	8b 7d 10             	mov    0x10(%ebp),%edi  # 获取变量 offset
{
    7ce4:	53                   	push   %ebx
	end_pa = pa + count;
    7ce5:	8b 75 0c             	mov    0xc(%ebp),%esi   # 获取变量 count
{
    7ce8:	8b 5d 08             	mov    0x8(%ebp),%ebx   # 获取变量 pa
	offset = (offset / SECTSIZE) + 1;
    7ceb:	c1 ef 09             	shr    $0x9,%edi         # offset 的值右移9位
	end_pa = pa + count;
    7cee:	01 de                	add    %ebx,%esi         # %esi = end_pa = pa + count
	offset = (offset / SECTSIZE) + 1;
    7cf0:	47                   	inc    %edi              # offset + 1
	pa &= ~(SECTSIZE - 1);
    7cf1:	81 e3 00 fe ff ff    	and    $0xfffffe00,%ebx  # 0xfffffe00 = ~(SECTSIZE - 1)
	while (pa < end_pa) {
    7cf7:	39 f3                	cmp    %esi,%ebx
    7cf9:	73 12                	jae    7d0d <readseg+0x31>   # while 中的条件不满足则跳转
		readsect((uint8_t*) pa, offset);
    7cfb:	57                   	push   %edi           #readsect() 的第二个参数
    7cfc:	53                   	push   %ebx           #readsect() 的第一个参数
		offset++;
    7cfd:	47                   	inc    %edi            # offset + 1
		pa += SECTSIZE;
    7cfe:	81 c3 00 02 00 00    	add    $0x200,%ebx     # pa += 512
		readsect((uint8_t*) pa, offset);
    7d04:	e8 73 ff ff ff       	call   7c7c <readsect>  # 调用 readsect 函数
		offset++;
    7d09:	58                   	pop    %eax            # pop readsect() 的第一个参数
    7d0a:	5a                   	pop    %edx            # pop readsect() 的第二个参数
    7d0b:	eb ea                	jmp    7cf7 <readseg+0x1b>  # 跳转，继续while循环
}
    7d0d:	8d 65 f4             	lea    -0xc(%ebp),%esp
    7d10:	5b                   	pop    %ebx
    7d11:	5e                   	pop    %esi
    7d12:	5f                   	pop    %edi
    7d13:	5d                   	pop    %ebp
    7d14:	c3                   	ret    

```



**Questions：**

- 在什么地方，处理器开始执行32位的代码，什么条件导致了从16位到32位的转换？	


​      在引导程序执行到 boot/main.c:60 行时会执行 call   *0x10018 ， 该指令执行后，处理器开始执行32位的代码。该转换的条件是引导程序加载内核完毕（即boot/main.c:53 中的for循环中的条件）。

- 引导加载程序最后执行的指令和内核的第一条指令分别是什么？

  引导加载程序的最后一条指令：`0x7d6b:	call   *0x10018`

  内核的第一条指令：`0x10000c:	movw   $0x1234,0x472`

- 内核的第一条指令在哪里？

  从加载程序的最后一条指令`0x7d6b:    call   *0x10018`可以看出， 内核第第一条指令在内存 `0x10018`处。

- 引导加载程序如何决定从磁盘中读取多少个扇区以获取整个内核，该信息是从何处获取到的？

  boot/main.c:52 行中的 eph 变量的值决定读取扇区的数量，该信息从内存`0x1002c` 处获取。




#### Exercise 4.

略



#### Exercise 5.

> 再次跟踪引导加载程序最初的几条指令并确定如果引导程序的链接地址错误，将执行的中断或其他错误操作的第一条指令。然后改变 boot/Makefrag 中的链接地址，执行 `make clean` 后重新编译，并再次跟踪引导加载程序的指令，查看会发生什么。
>

将 Makefrag中的 `-Ttext 0x7C00`修改为 `-Ttext 0x7C10` ，引导加载程序会在 `ljmp    $PROT_MODE_CSEG, $protcseg` 命令处出错，检查该处的后续指令为：

```shell
(gdb) x/5i 0x7c42               # wrong
   0x7c42:	jl     0x7c44
   0x7c44:	add    %ch,%al
   0x7c46:	lret   
   0x7c47:	add    %al,(%bx,%si)
   0x7c49:	add    %ch,%bl
(gdb) 

```

正确的代码为：

```assembly
7c42:   66 b8 10 00             mov    $0x10,%ax
7c46:   8e d8                   mov    %eax,%ds
7c48:   8e c0                   mov    %eax,%es
7c4a:   8e e0                   mov    %eax,%fs
7c4c:   8e e8                   mov    %eax,%gs
7c4e:   8e d0                   mov    %eax,%ss
```

**TODO：为什么会出现这种现象呢？**

将链接地址修改后，程序载入内存后将从 0x7c10 处开始执行。但通过objdump命令查看发现，其加载地址也改变为0x7c10，为什么ljmp跳转时会出错呢？

```shell
ubuntu@VM-20-12-ubuntu:~/WorkSpace/S6.828/lab$ objdump -h obj/boot/boot.out 
obj/boot/boot.out:     file format elf32-i386
Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         00000186  00007c10  00007c10  00000074  2**2
              CONTENTS, ALLOC, LOAD, CODE
  1 .eh_frame     000000a8  00007d98  00007d98  000001fc  2**2
              CONTENTS, ALLOC, LOAD, READONLY, DATA

  2 .stab         0000087c  00000000  00000000  000002a4  2**2
              CONTENTS, READONLY, DEBUGGING

  3 .stabstr      00000925  00000000  00000000  00000b20  2**0
              CONTENTS, READONLY, DEBUGGING

  4 .comment      00000029  00000000  00000000  00001445  2**0
              CONTENTS, READONLY


```



#### Exercise 6.

> 重启QEMU/GDB，在刚进入引导程序时，查看 0x00100000 处的8个word；然后再在由引导程序进入内核时查看该处内存的内容，查看有何区别，为什么会有这种区别？

在刚进入引导程序时查看 `0x00100000` 处的8个word如下所示，可以看到此时该处的内容均为0.

```shell
The target architecture is assumed to be i8086
[f000:fff0]    0xffff0:	ljmp   $0xf000,$0xe05b
0x0000fff0 in ?? ()

- symbol-file obj/kern/kernel
  (gdb) b *0x7c00
  Breakpoint 1 at 0x7c00
  (gdb) c
  Continuing.
  [   0:7c00] => 0x7c00:cli    

Breakpoint 1, 0x00007c00 in ?? ()
(gdb) x/8x 0x00100000 
0x100000:	0x00000000	0x00000000	0x00000000	0x00000000
0x100010:	0x00000000	0x00000000	0x00000000	0x00000000
(gdb) 
```

在 `0x7d6b:  call   *0x10018` 处设置断点（此时已从磁盘读取内核完成），查看前述地址处的内容如下所示：

```shell
(gdb) x/8x 0x00100000 
0x100000:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0x100010:	0x34000004	0x2000b812	0x220f0011	0xc0200fd8
(gdb) 
```

这是因为，刚进入引导程序时还没有将内核读入内存中，所以内核的起始地址 `0x00100000`  处没有值，当读取内核完毕时，内核的相关代码、数据等已存储在起始地址处。



## Part 3: The Kernel



#### Exercise 7.

> 跟踪内核代码并在 `movl %eax, %cr0` 处停止，查看内存 `0x00100000` 和  `0xf0100000` 处的内容。然后单步执行。然后再查看这两处内存的内容，确保理解发生了什么。
>
> 在建立新的映射后，如果映射错误，不能正常工作的第一条指令是什么？

```shell
(gdb) si
=> 0x100025:	mov    %eax,%cr0
0x00100025 in ?? ()
(gdb) x/8x 0x00100000
0x100000:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0x100010:	0x34000004	0x2000b812	0x220f0011	0xc0200fd8
(gdb) x/8x 0xf0100000
0xf0100000 <_start+4026531828>:	0x00000000	0x00000000	0x00000000	0x00000000
0xf0100010 <entry+4>:	0x00000000	0x00000000	0x00000000	0x00000000
(gdb) si
=> 0x100028:	mov    $0xf010002f,%eax
0x00100028 in ?? ()
(gdb) x/8x 0xf0100000
0xf0100000 <_start+4026531828>:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0xf0100010 <entry+4>:	0x34000004	0x2000b812	0x220f0011	0xc0200fd8
(gdb) 
```

由上述代码可见，在执行完 `mov  %eax,%cr0` 指令后， `0xf0100000` 出的内容与 `0x00100000` 处的完全相同。CR0的第31位是分页允许位(Paging Enable)，它表示芯片上的分页部件是否允许工作。`mov  %eax,%cr0` 指令的作用应该就是开启分页功能，此后 `0x00100000` 处的地址映射到了 `0xf0100000` 处。

如果将 `mov  %eax,%cr0` 指令注释掉，重新运行后执行该指令的后续指令会使qemu core dumped。

#### Exercise 8.

> 我们省略了一小部分的代码，该代码需要通过控制符“%o”打印八进制的数字。找到并补充完整这部分代码。

并回答以下问题：

1. 解释printf.c 和 console.c 的接口。具体地说，console.c 导出的接口是什么？printf.c 是如何使用这些接口的？

   ​	printf.c 中用来调用console.c 的功能是通过 printf.c : putch() 函数调用 console.c : cputchar() 函数来实现的。在printf.c 中的其他函数中，通过传递 putch() 函数的函数指针来调用相应的功能。在console.c 的 cputchar() 函数中会调用console.c 的其他功能来实现在命令行窗口的字符打印功能。

2. 解释console.c 中的如下代码：

```c
1      if (crt_pos >= CRT_SIZE) { // CRT_SIZE = 80 * 25
2              int i;  // 将crt_buf 的 81 - 80 * 25 个字符复制到 0 - 80 * 24 处
3              memmove(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16_t));
4              for (i = CRT_SIZE - CRT_COLS; i < CRT_SIZE; i++) 
5                      crt_buf[i] = 0x0700 | ' ';  // 将后crt_buf 的后80个字符设为 0x0720
6              crt_pos -= CRT_COLS;				  // crt_pos 自减 80
7      }
```

​	

3. 对于下述问题你可以先参考 Lecture2 中的笔记。这些笔记包括了 GCC 在 x86 平台上的调用约定。

单步执行跟踪下述代码：

```c
int x = 1, y = 3, z = 4;
cprintf("x %d, y %x, z %d\n", x, y, z);
```

- 在 cprintf() 函数的调用中， fmt指向什么？ap 指向什么？

  ​	fmt 	指向 cprintf() 的第一个参数，即字符串，ap指向栈中的参数x。

- 列出（按执行顺序）cons_putc, va_arg 和 vcprintf 的所有调用。对 cons_putc ， 列出其参数。对 va_arg ，理出调用前后 ap 指向什么。对vcprintf 列出其两个参数的值。

  ​	执行顺序：

  ​		vcprintf() -> vprintfmt()

  ​		cons_putc() : 2次    (“x ")   此时 fmt 指向 d

  ​		getint -> va_arg()  

  ​		cons_putc()    ： 打印 1

  ​		cons_putc()    ： 打印 “， y ”

  ​		getuint -> va_arg()   

  ​		cons_putc()    ： 打印 3

  ​		cons_putc()    ： 打印 “， z ”

  ​		getuint -> va_arg()   

  ​		cons_putc()    ： 打印 4



4. ​

```c
    unsigned int i = 0x00646c72;
    cprintf("H%x Wo%s", 57616, &i);
```

该代码的输出是什么？通过单步执行的方式，解释如何得到这个输出。[ASCII Table](https://www.asciitable.com/)

该代码的输出依赖于x86的小端顺序。如果x86替换为大端序，如何修改 i 的值使其得到相同的输出？需要修改 57616 的值么？

输出为：  `H0000e110 World`

如替换为大端序，需修改   `unsigned int i = 0x726c6400;`  不需要修改 57616 的值。



5. 在以下代码中，‘y=’ 后会打印什么？（注意：答案不是一个特定值。）为什么会这样？

```c
  cprintf("x=%d y=%d", 3);
```
输出y的数值时，ap 会指向 3 所在的内存之上的内存，所有y= 后面的数值依赖于该内存运行时存放的数据。

6. 假设GCC改变了它的调用规则，以至它会按声明顺序将参数推入到栈中，所以最后一个参数会最后推入栈中。如何修改 cprintf 或它的接口以至于该函数仍然有可能传递变量数量的参数。

直接把参数列表反过来？


**挑战：**

> 增强控制台使其可以用不同的颜色来显示文本。传统的方法是让它解释嵌入到打印到控制台的文本字符串中的ANSI转义序列，但你可以选用任意你喜欢的机制。在参考页和web的其他地方有大量关于VGA显示硬件的变成资料。如果您觉得很冒险，可以尝试将VGA硬件切换到图形模式，并让控制台将文本绘制到图形帧缓冲区。



### The Stack

#### Exercise 9.

> 找到内核在何处初始化栈，并确定栈的内存地址。内核是如何为栈顶预留空间的？初始栈顶指针指向的是这段空间的哪一端？
>

内核的entry.S 中预留了一个从   f0110000 - f0108000  的段，该段用于栈的使用空间。

entry.S : 77 处初始化栈寄存器 esp，此时栈顶指针指向高地址端，即 f0110000  处。



#### Exercise 10.

> 熟悉x86平台上的C语言的函数调用规则，找到 obj/kernel.asm 中 test_backtrace() 函数的地址，并在此处设置断点，并检查内核启动后每次调用该函数都发生了什么。在test_backtrace（） 的每次递归调用中，在栈上推入了几个4字节的数据，这些数据分别是什么？

先在kern/init.c 的i386_init（）函数中增添一个函数体为空的函数，查看该函数进出期间的栈上变化。

函数内容如下所示：

```c
mon_backtrace(1, 0, 0);   // 调用
    
int mon_backtrace(int argc, char **argv, struct Trapframe *tf)   // 定义
{
	return 0;
}
```



```shell
=> 0xf0100136 <i386_init+144>:	push   $0x0           # 函数第三个参数入栈

=> 0xf0100138 <i386_init+146>:	push   $0x0

=> 0xf010013a <i386_init+148>:	push   $0x1			# 第一个参数入栈

=> 0xf010013c <i386_init+150>:	call   0xf01008d0 <mon_backtrace>  
# eip值入栈，入栈值为原流程中的该指令的后一条指令的地址值

=> 0xf01008d0 <mon_backtrace>:	push   %ebp			# 前一函数的栈基址入栈
	
=> 0xf01008d1 <mon_backtrace+1>:	mov    %esp,%ebp # 将当前栈地址赋给 ebp 

=> 0xf01008d3 <mon_backtrace+3>:	mov    $0x0,%eax

=> 0xf01008d8 <mon_backtrace+8>:	pop    %ebp       # 弹出迁移函数的基址值给 ebp

=> 0xf01008d9 <mon_backtrace+9>:	ret    			# 函数调用结束，从栈中取出eip值

=> 0xf0100141 <i386_init+155>:	add    $0x10,%esp

=> 0xf0100144 <i386_init+158>:	sub    $0xc,%esp

=> 0xf0100147 <i386_init+161>:	push   $0x0
```

在该函数的调用中栈上push 入 4 个4字节数据（3个函数参数，一个前一函数的栈基址），1个2字节数据（call指令中push入的eip值，为函数调用结束后要执行的指令地址）。



#### Exercise 11. 

> 按照要求补全mon_backtrace函数。使用 `make grade` 命令查看分数。

详见 lab1 代码。



#### Exercise 11.

> 修改栈回溯函数，对于每个 eip ， 增加函数名称，源文件名和行号。
>
> 在 `debuginfo_eip` 中，`__STAB_*` 是来自哪里的？
>
> 通过插入 `stab_binsearch()` 函数的调用，找到对应每个 eip 地址的行号，完善 `debuginfo_eip（）` 函数的实现 。
>
> 向内核的 monitor 增加一个backtrace 函数，扩展 `mon_backtrace`  的实现通过调用 `debuginfo_eip` 来打印如下所示的栈信息：
>
> ```
> K> backtrace
> Stack backtrace:
>   ebp f010ff78  eip f01008ae  args 00000001 f010ff8c 00000000 f0110580 00000000
>          kern/monitor.c:143: monitor+106
>   ebp f010ffd8  eip f0100193  args 00000000 00001aac 00000660 00000000 00000000
>          kern/init.c:49: i386_init+59
>   ebp f010fff8  eip f010003d  args 00000000 00000000 0000ffff 10cf9a00 0000ffff
>          kern/entry.S:70: <unknown>+0
> K> 
> ```

stab的参考资料可查看这个链接。http://www.math.utah.edu/docs/info/stabs_1.html



backtrace 函数的实现步骤：

在debuginfo_eip z中加入如下两行代码，

```c
stab_binsearch(stabs, &lline, &rline, N_SLINE, addr);
info->eip_line = stabs[lline].n_desc;
```



在monitor.h 和 .c 中增加 backtrace 函数的声明和实现：

```c
int backtrace(int argc, char **argv, struct Trapframe *tf)
{
    cprintf("Stack backtrace:\n");
    uint32_t *ebp = (uint32_t *)read_ebp();
    while (ebp)
    {
        cprintf("ebp %08x  eip %08x  args %08x %08x %08x %08x %08x\n",
                ebp,
                *(ebp + 1),
                *(ebp + 2),
                *(ebp + 3),
                *(ebp + 4),
                *(ebp + 5),
                *(ebp + 6));
        struct Eipdebuginfo eip_info;
        debuginfo_eip(*(ebp + 1), &eip_info);
        cprintf("\t%s:%d: %.*s+%d\n", 
                    eip_info.eip_file, eip_info.eip_line, 
                    eip_info.eip_fn_namelen, eip_info.eip_fn_name, 
                    *(ebp+1) - eip_info.eip_fn_addr);
        ebp = (uint32_t *)*ebp;
    }
    return 0;
}
```

修改 test_backtrace 函数：

```c
void test_backtrace(int x)
{
	cprintf("entering test_backtrace %d\n", x);
	if (x > 0)
		test_backtrace(x-1);
	else
        backtrace(0, 0, 0);
	cprintf("leaving test_backtrace %d\n", x);
}
```













