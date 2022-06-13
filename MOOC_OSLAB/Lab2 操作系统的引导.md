# 操作系统的引导

## 1 实验介绍

**实验目的：**

- 熟悉 hit-oslab 实验环境；
- 建立对操作系统引导过程的深入认识；
- 掌握操作系统的基本开发过程；
- 能对操作系统代码进行简单的控制，揭开操作系统的神秘面纱。





**实验内容：**

1. 阅读《Linux 内核完全注释》的第 6 章，对计算机和 Linux 0.11 的引导过程进行初步的了解；
2. 按照下面的要求改写 0.11 的引导程序 bootsect.s
3. 有兴趣同学可以做做进入保护模式前的设置程序 setup.s。

改写 `bootsect.s` 主要完成如下功能：

1. bootsect.s 能在屏幕上打印一段提示信息“XXX is booting...”，其中 XXX 是你给自己的操作系统起的名字，例如 LZJos、Sunix 等（可以上论坛上秀秀谁的 OS 名字最帅，也可以显示一个特色 logo，以表示自己操作系统的与众不同。）

改写 `setup.s` 主要完成如下功能：

1. bootsect.s 能完成 setup.s 的载入，并跳转到 setup.s 开始地址执行。而 setup.s 向屏幕输出一行"Now we are in SETUP"。
2. setup.s 能获取至少一个基本的硬件参数（如内存参数、显卡参数、硬盘参数等），将其存放在内存的特定地址，并输出到屏幕上。
3. setup.s 不再加载 Linux 内核，保持上述信息显示在屏幕上即可。





在实验报告中回答如下问题：

1. 有时，继承传统意味着别手蹩脚。x86 计算机为了向下兼容，导致启动过程比较复杂。请找出 x86 计算机启动过程中，被硬件强制，软件必须遵守的两个“多此一举”的步骤（多找几个也无妨），说说它们为什么多此一举，并设计更简洁的替代方案。




## 2 实验完成步骤

完整版代码详见实验代码的lab2 分支。

### 2.1 改写 bootsect.s

首先，修改 boot/bootsect.s 中的如下内容：

```assembly
msg1:
    .byte 13,10
    .ascii "Loading system ..."   ;将这里的字符串修改成自己想要的样子
    .byte 13,10,13,10
```



修改下述代码段：

```assembly
! Print some inane message

	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	
	mov	cx,#30			!这里的数值要修改为字符串的长度 + 6
	mov	bx,#0x0007		! page 0, attribute 7 (normal)
	mov	bp,#msg1
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
```



### 2.2 改写 stepup.s



首先新增如下定义：

```assembly
msg1:
	.byte 13,10
	.ascii "Now we are in SETUP"
	.byte 13,10,13,10
msg_pos:
    .byte 13,10
    .ascii "Cursor POS:"
msg_mem:
    .byte 13,10
    .ascii "Memory SIZE:"
msg_cyl:
    .byte 13,10
    .ascii "Cyls:"
msg_head:
    .byte 13,10
    .ascii "Heads:"
msg_sectors:
    .byte 13,10
    .ascii "Sectors:"
```

随后，编写命令，打印各项参数：

```assembly
! Check that there IS a hd1 :-)
print_hard_message:

! init ss:sp
    mov ax,#INITSEG
    mov ss,ax
    mov sp,#0xFF00

! Cursor pos
    mov ax, #SETUPSEG	;这里是将数据段地址设为字符串所在的段
    mov es, ax
    mov ds, ax
	mov	cx,#13			;打印 Cursor POS:
	mov	bx,#0x0007	
	mov	bp,#msg_pos
	mov	ax,#0x1301
	int	0x10			;调用BIOS中断，打印字符

    mov ax, #INITSEG	  ;这里切换为 0x90000 所在的段，准备打印硬件参数
    mov es, ax
    mov ds, ax
    mov dx, [0]			; 将ds:[0] 内存出的值存入dx寄存器
    call print_hex		; 调用函数打印dx中的值

! Memory
    mov ax, #SETUPSEG
    mov es, ax
    mov ds, ax

	mov	cx,#14
	mov	bx,#0x0007	
	mov	bp,#msg_mem
	mov	ax,#0x1301
	int	0x10

    mov ax, #INITSEG
    mov es, ax
    mov ds, ax
    mov dx, [2]
    call print_hex
! Cyles
    mov ax, #SETUPSEG
    mov es, ax
    mov ds, ax

	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10

	mov	cx,#7
	mov	bx,#0x0007	
	mov	bp,#msg_cyl
	mov	ax,#0x1301
	int	0x10

    mov ax, #INITSEG
    mov es, ax
    mov ds, ax
    mov dx, [4]
    call print_hex
!Heads
    mov ax, #SETUPSEG
    mov es, ax
    mov ds, ax	

    mov	ah,#0x03
	xor	bh,bh
	int	0x10

	mov	cx,#8
	mov	bx,#0x0007	
	mov	bp,#msg_head
	mov	ax,#0x1301
	int	0x10

    mov ax, #INITSEG
    mov es, ax
    mov ds, ax
    mov dx, [6]
    call print_hex

!Secotrs
    mov ax, #SETUPSEG
    mov es, ax
    mov ds, ax

    mov	ah,#0x03
	xor	bh,bh
	int	0x10

	mov	cx,#10
	mov	bx,#0x0007	
	mov	bp,#msg_sectors
	mov	ax,#0x1301
	int	0x10

    mov ax, #INITSEG
    mov es, ax
    mov ds, ax
    mov dx, [12]
    call print_hex
```



