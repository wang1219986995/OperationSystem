# Homework: xv6 CPU alarm



在本练习中，你将为xv6增加一个在进程使用CPU期间周期性报警的特性 。这对一些使用CPU受限的计算进程或进行计算的同时进行周期性操作的进程是有用的。更一般的说，你会实现一个较原始的用户级的中断/错误处理程序；例如，你可以使用类似的方式去处理程序中的叶错误。

你应该新增一个 alarm(interval, handler) 的系统调用。如果一个程序调用 alarm(n, fn) , 然后在程序每使用 n 个CPU时间片之后，函数 fn会被调用。当 函数 fn 返回后，程序将从停止的地方继续执行。在xv6中一个时间片是一个任意的时间单位，具体时间取决于计时器硬件多久产生一次中断。

你应该将以下示例程序放到 alarmtest.c 中：

```c
#include "types.h"
#include "stat.h"
#include "user.h"

void periodic();

int
main(int argc, char *argv[])
{
  int i;
  printf(1, "alarmtest starting\n");
  alarm(10, periodic);
  for(i = 0; i < 25*500000; i++){
    if((i % 250000) == 0)
      write(2, ".", 1);
  }
  exit();
}

void
periodic()
{
  printf(1, "alarm!\n");
}
```

程序调用 alarm(10, periodic) 告诉内核每10个时间片强制调用periodic一次，并旋转一段时间。当你实现了内核的 alarm（） 的系统调用后，alarmtest应该产生如下所示的输出：

```shell
$ alarmtest
alarmtest starting
.....alarm!
....alarm!
.....alarm!
......alarm!
.....alarm!
....alarm!
....alarm!
......alarm!
.....alarm!
...alarm!
...$ 
```

（如果你只看到了一个 “alaram！” ， 尝试将 alarmtest.c 中的迭代次数增加十倍。）

提示：

你需要修改 Makefile 以使 alarmtest.c 被编译为一个xv6 用户程序。

放入 user.h 中的正确的声明是：

```c
 int alarm(int ticks, void (*handler)());
```

你同样需要更新 syscall.h 和 usys.S 来允许 alarmtest 来调用 alarm 系统调用。

你的 sys_alarm() 应该在 proc 结构体的新字段中保存报警间隔和指向处理函数的指针，查看proc.h 。

这有一个面费的 sys_alarm():

```c
    int
    sys_alarm(void)
    {
      int ticks;
      void (*handler)();

      if(argint(0, &ticks) < 0)
        return -1;
      if(argptr(1, (char**)&handler, 1) < 0)
        return -1;
      myproc()->alarmticks = ticks;
      myproc()->alarmhandler = handler;
      return 0;
    }
```

将上述函数加入到 syscall.c 中，并在该文件的 syscalls 数组中增加 SYS_ALARM 入口。

- 你需要跟踪从最后一次调用（或在下一次调用之前）到进程的报警处理函数经过了多少个时间片。你需要在 struct proc 中增加一个新字段。你可以在proc.c 的 allocproc() 中初始化 proc 的字段。

- 每个时间片，硬件时间都会强制进行一次中的，会在 trap() 中通过 case T_IRQ0 + IRQ_TIME 进行处理，你应该在这里增加一些代码。

只有当进程正在运行且计时器终端来自用户空间的情况下，你才想执行进程报警，类似于下面的判断：

```c
if(myproc() != 0 && (tf->cs & 3) == 3) ...
```

在你的 IRQ_TIME 代码中，当一个进程的报警周期过期时，你需要让他执行他的报警处理函数。你如何实现呢？

你需要进行安排以使处理函数返回时，进程从停止的地方开始执行。如何实现呢？

你可以在 alarmtest.asm 中看到alarmtest程序的汇编代码。

如果设置qemu只使用一个CPU，使用gdb查看traps时会更简单，设置方法：

```
 make CPUS=1 qemu
```

如果你的解决方案在执行处理程序时没有保存调用者保存的用户寄存器也是可以的。

**可选挑战：**

1. 在调用处理程序前后，保存和恢复调用者保存的用户寄存器。
2. 避免处理程序的重复调用， 如果一个处理程序的调用还没有返回，不要再次调用。
3. 假设你的代码没有检查 tf->esp 是否有效，实现一个内核安全攻击，利用你的警报处理程序的代码。