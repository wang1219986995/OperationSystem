1. => 0x7c43:	mov    $0x7c00,%esp
2. 进入bootmain后会执行 push %ebp 指令，该指令是将寄存器 ebp的值入栈，该值为0.



0x7c00   ---------------

​		16字节

0x7bf0   ---------------

​		16字节

0x7be0   ---------------

​		4字节

0x7bdc    ---------------

当指令执行到 `0x10000c:	mov    %cr4,%eax` 处时，esp 指向 0x7dbc处，此时栈中共存有36字节的数据。此时 `x/24x $esp` 的输出如下所示：

```shell
(gdb) x/24x $esp
0x7bdc:	0x00007d8d	0x00000000	0x00000000	0x00000000	 # 此行都是栈中数据 
0x7bec:	0x00000000	0x00000000	0x00000000	0x00000000	 # 此行都是栈中数据 
0x7bfc:	0x00007c4d	0x8ec031fa	0x8ec08ed8	0xa864e4d0   # 此行第一个是栈中数据
0x7c0c:	0xb0fa7502	0xe464e6d1	0x7502a864	0xe6dfb0fa
0x7c1c:	0x16010f60	0x200f7c78	0xc88366c0	0xc0220f01
0x7c2c:	0x087c31ea	0x10b86600	0x8ed88e00	0x66d08ec0
```

栈中的非 0 数据有两个，分别为：

①  内存   0x7bfc -  0x7c00  中存储的  0x00007c4d    

该处数据来自于一下代码

```assembly
  # Set up the stack pointer and call into C.
  movl    $start, %esp
    7c43:	bc 00 7c 00 00       	mov    $0x7c00,%esp
  call    bootmain    # 此处call指令将eip入栈，该值为call之后的指令的地址，即 7c4d 
    7c48:	e8 ee 00 00 00       	call   7d3b <bootmain>

  # If bootmain returns (it shouldn't), trigger a Bochs
  # breakpoint if running under Bochs, then loop.
  movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
    7c4d:	66 b8 00 8a          	mov    $0x8a00,%ax
  movw    %ax, %dx
    7c51:	66 89 c2             	mov    %ax,%dx
  outw    %ax, %dx
    7c54:	66 ef                	out    %ax,(%dx)
  movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
    7c56:	66 b8 e0 8a          	mov    $0x8ae0,%ax
  outw    %ax, %dx
    7c5a:	66 ef                	out    %ax,(%dx)
```

② 内存   0x7bdc - 0x7be0 中存储的  0x00007d8d ， 该处数据来自于bootmain中的以下指令：

```assembly
7d87:   ff 15 18 00 01 00  call   *0x10018  # 此处将eip入栈，该值为下条指令的地址 7d8d

7d8d:   eb d5    jmp    7d64 <bootmain+0x29>
```



