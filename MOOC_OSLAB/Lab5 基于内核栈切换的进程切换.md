# 基于内核栈切换的进程切换

实验5 链接 https://www.lanqiao.cn/courses/115/learning/?id=571

现在的 Linux 0.11 采用 TSS（后面会有详细论述）和一条指令就能完成任务切换，虽然简单，但这指令的执行时间却很长，在实现任务切换时大概需要 200 多个时钟周期。

而通过堆栈实现任务切换可能要更快，而且采用堆栈的切换还可以使用指令流水的并行优化技术，同时又使得 CPU 的设计变得简单。所以无论是 Linux 还是 Windows，进程/线程的切换都没有使用 Intel 提供的这种 TSS 切换手段，而都是通过堆栈实现的。

本次实践项目就是将 Linux 0.11 中采用的 TSS 切换部分去掉，取而代之的是基于堆栈的切换程序。具体的说，就是将 Linux 0.11 中的 `switch_to` 实现去掉，写成一段基于堆栈切换的代码。

本次实验包括如下内容：

- 编写汇编程序 `switch_to`：
- 完成主体框架；
- 在主体框架下依次完成 PCB 切换、内核栈切换、LDT 切换等；
- 修改 `fork()`，由于是基于内核栈的切换，所以进程需要创建出能完成内核栈切换的样子。
- 修改 PCB，即 `task_struct` 结构，增加相应的内容域，同时处理由于修改了 task_struct 所造成的影响。
- 用修改后的 Linux 0.11 仍然可以启动、可以正常使用。
- （选做）分析实验 3 的日志体会修改前后系统运行的差别。


## 1 实验内容

### 1.1 编写 switch_to

**修改  `kernal/sched.c`** 

将目前的 `schedule()` 函数（在 `kernal/sched.c` 中）做稍许修改，将下面的代码：

```c
if ((*p)->state == TASK_RUNNING && (*p)->counter > c)
    c = (*p)->counter, next = i;

//......

switch_to(next);
```

修改为：

```c
extern int timer_interrupt(void);
extern int system_call(void);
extern long switch_to(struct task_struct *p, unsigned long address); // 这一行是新加的

union task_union {
	struct task_struct task;
	char stack[PAGE_SIZE];
};

//......

void schedule(void)
{
    //......
    // 这一句是新加的
	struct task_struct * pnext = &(init_task.task);struct task_struct * pnext;
	//.......
	if ((*p)->state == TASK_RUNNING && (*p)->counter > c)
    	c = (*p)->counter, next = i, pnext = *p;	// 这一行修改了
	//.......
	switch_to(pnext, LDT(next));
}
```



**编写  `switch_to`** 

实现 `switch_to` 是本次实践项目中最重要的一部分。

由于要对内核栈进行精细的操作，所以需要用汇编代码来完成函数 `switch_to` 的编写。

首先在`kernel/system_call.s` 中增加ESP0 和 KERNEL_STACK 的定义：

```assembly
!......
CS		= 0x20
EFLAGS		= 0x24
OLDESP		= 0x28
OLDSS		= 0x2C

! 定义变量值
ESP0 = 4
KERNEL_STACK = 12
state	= 0		# these are offsets into the task-struct.
counter	= 4
priority = 8
signal	= 12
!......
```

修改 task_struct 的定义：

```c
struct task_struct {
/* these are hardcoded - don't touch */
	long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	long counter;
	long priority;
    long kernelstack;	// 新增 kernelstack 字段
	long signal;
	//......
}
```

修改 INIT_TASK 定义：

```
//修改前
/* state etc */	{ 0,15,15, \
//......


//修改后
#define INIT_TASK \
/* state etc */	{ 0,15,15,PAGE_SIZE+(long)&init_task, \
//......
```



在`kernel/system_call.s` 中 `switch_to` 函数的定义：

```assembly
switch_to:
    pushl %ebp
    movl %esp,%ebp
    pushl %ecx
    pushl %ebx
    pushl %eax
    movl 8(%ebp),%ebx
    cmpl %ebx,current
    je 1f
!PCB切换
    movl %ebx,%eax
    xchgl %eax,current  

! TSS中的内核栈指针的重写
    movl tss,%ecx
    addl $4096,%ebx
    movl %ebx,ESP0(%ecx)

! 切换内核栈
	movl %esp,KERNEL_STACK(%eax)
	movl 8(%ebp),%ebx
	movl KERNEL_STACK(%ebx),%esp	
! 切换LDT
    mov 12(%ebp), %ecx
    lldt %cx

! 重置fs寄存器
    movl $0x17,%ecx
    mov %cx,%fs

! 和后面的 clts 配合来处理协处理器
    cmpl %eax,last_task_used_math
    jne 1f
    clts

1:  popl %eax
    popl %ebx
    popl %ecx
    popl %ebp
    ret
```



### 1.2 修改 fork



```c
extern long first_return_from_kernel(void);

//......

int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
{
	struct task_struct *p;
	int i;
	struct file *f;

	p = (struct task_struct *) get_free_page();
	if (!p)
		return -EAGAIN;
	task[nr] = p;
	*p = *current;	/* NOTE! this doesn't copy the supervisor stack */
	p->state = TASK_UNINTERRUPTIBLE;
	p->pid = last_pid;
	p->father = current->pid;
	p->counter = p->priority;
	p->signal = 0;
	p->alarm = 0;
	p->leader = 0;		/* process leadership doesn't inherit */
	p->utime = p->stime = 0;
	p->cutime = p->cstime = 0;
	p->start_time = jiffies;

  	// 注释 tss 设置相关代码
	// p->tss.back_link = 0;
	// p->tss.esp0 = PAGE_SIZE + (long) p;
	// p->tss.ss0 = 0x10;
	// p->tss.eip = eip;
	// p->tss.eflags = eflags;
	// p->tss.eax = 0;
	// p->tss.ecx = ecx;
	// p->tss.edx = edx;
	// p->tss.ebx = ebx;
	// p->tss.esp = esp;
	// p->tss.ebp = ebp;
	// p->tss.esi = esi;
	// p->tss.edi = edi;
	// p->tss.es = es & 0xffff;
	// p->tss.cs = cs & 0xffff;
	// p->tss.ss = ss & 0xffff;
	// p->tss.ds = ds & 0xffff;
	// p->tss.fs = fs & 0xffff;
	// p->tss.gs = gs & 0xffff;
	// p->tss.ldt = _LDT(nr);
	// p->tss.trace_bitmap = 0x80000000;

    long *kernstack;
    kernstack = (long)(PAGE_SIZE + (long)p);
    // int 指令执行时入栈的内容
    *(--kernstack) = ss & 0xffff;
	*(--kernstack) = esp;
	*(--kernstack) = eflags;
	*(--kernstack) = cs & 0xffff;
	*(--kernstack) = eip;

    // 标号函数处的popl顺序
    *(--kernstack) = ds & 0xffff;
	*(--kernstack) = es & 0xffff;
	*(--kernstack) = fs & 0xffff;
	*(--kernstack) = gs & 0xffff;
	*(--kernstack) = esi;
	*(--kernstack) = edi;
	*(--kernstack) = edx;

    // switch_to 的出栈顺序
    *(--kernstack) = (long)first_return_from_kernel;
	*(--kernstack) = ebp;
	*(--kernstack) = ecx;
	*(--kernstack) = ebx;
	*(--kernstack) = 0;	/* 相当于eax=0 */
    p->kernelstack = kernstack;
  
	//......
  
}
```



在 system_call.s 中设置如下内容：

```
.globl switch_to, first_return_from_kernel

//......

.align 2
first_return_from_kernel:
	popl %edx
	popl %edi
	popl %esi
	pop %gs
	pop %fs
	pop %es
	pop %ds
	iret

```





## 2 用户态的进程切换

本节主要介绍进程在用户态下调用 `schedule()` 函数主动让出CPU的基于堆栈切换的进程切换过程。

 在`schedule()` 函数中，与进程切换密切相关的主要包括以下几行函数：

```c
void schedule(void)
{
	//......
	struct task_struct * pnext = &(init_task.task);
	//......

			if ((*p)->state == TASK_RUNNING && (*p)->counter > c)
              // 这里里将 pnext 设为下一个要执行的进程
				c = (*p)->counter, next = i, pnext = *p;
	
  	// 这里调用汇编的 switch_to 方法执行切换
	switch_to(pnext, _LDT(next));
}
```











































## 3 内核态的进程切换

本节主要介绍进程在内核中执行程序时让出CPU的情况。



