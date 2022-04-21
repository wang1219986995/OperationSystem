# HW4: xv6 lazy page allocation



## Part One: Eliminate allocation from sbrk()

删除 `sbrk(n)` 中的页分配系统调用的实现，该系统调用是由 `sysproc.c` 中的 `sys_sbrk()` 实现的。 `sbrk(n)` 系统调用增加进程n个自己的内存，并返回新分配区域的开始。你的新  `sbrk(n)`  的实现应该只增加进程内存n个字节的大小（myproc()->sz）。这个过程并不分配内存，所以你应该删除 `grwoproc()` 的调用（但仍需要增加内存的大小）。

尝试猜想一下这个修改会导致什么结果：什么会崩溃？

完成修改后，启动xv6，并在命令行中输入 `echo hi` ，你应该看到如下信息：

```shell
init: starting sh
$ echo hi
pid 3 sh: trap 14 err 6 on cpu 0 eip 0x12f1 addr 0x4004--kill proc
$ 
```

该信息来自 `trap.c` 中的内核陷阱处理程序；上述命令产生了一个页错误（trap 14 ，或 T_PGFLT），这个xv6内核不知道如何处理。确保你已经理解了错误为什么会发生。`addr 0x4004` 表示导致该错误的虚拟地址是 `0x4004` 。

解答：

修改 sys_sbrk() 函数如下所示：

```c
int sys_sbrk(void)
{
  int addr;
  int n;
  if(argint(0, &n) < 0)
     return -1;
  addr = myproc()->sz;
  myproc()->sz += n;
  return addr;
}
```

修改后xv6 shell的输出：

```shell
$ echo hi
pid 3 sh: trap 14 err 6 on cpu 0 eip 0x112c addr 0x4004--kill proc
```





## Part Two: Lazy allocation

修改 trap.c 中的代码，通过在页错误的地址处映射新分配的页到物理内存，来响应用户空间中的页错误，然后返回用户空间让进程继续执行。你应该在输出“pid 3 sh：trap 14” 的 printf 语句之前增加代码。你的代码不要求覆盖所有的极端用例，只需好到可以运行简单的命令，例如 `echo ， ls` 等。

**提示：**

- 查看 `cprintf` 的参数，了解如何找到导致页错误的虚拟地址。



- 从 `vm.c` 中的 `allocuvm()` 中获取代码，该函数是`sbrk()` 通过 `grwoproc()` 调用的。



- 使用 `PGROUNDDOWN(va)` 将出错的虚拟地址舍入到页边界。



-  `break` 或 `return`，以避免 `cprintf` 和  `myproc()->killed = 1`



- 你需要调用 `mappages()` 。 为了调用该函数，你需要删除 `vim.c` 中 `mappages()` 函数的 `static` 修饰，然后在`trap.c` 中声明该函数。



- 你可以在 `trap()` 函数中检查 `tf->trapno` 是否等于 `T_PGFLT` 来判断是否是页错误。




完成以上工作后，你的缓式分配代码应该在 echo hi 的运行中起作用。你应该在shell中至少获取一个页错误（并因此缓式分配内存），可能有两个。

解答：

在 trap() 函数中新增如下部分代码：

```c
    if(tf->trapno == T_PGFLT)
    {
        uint addr = PGROUNDDOWN(rcr2());
        char *mem = kalloc();
        if(mem == 0)
        {
            cprintf("allocuvm out of memory.\n");
        }
        memset(mem, 0, PGSIZE);
        pde_t* pgdir = myproc()->pgdir;
        if(mappages(pgdir, (char*)addr, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0)
        {
            cprintf("allocuvm out of memory (2)\n");
            kfree(mem);
        }
        break;;
    }
```

这里没有循环分配内存至 myproc()->sz 是因为这里是缓式分配内存，只有出现页错误时，才说明对应的内存被使用了。而其他的小于 myproc()->sz 部分的内存可能是暂时未用到的，未使用之前不进行分配。



另外，这不是完整正确的实现。以下是我们所知的一些问题，请参阅以下挑战。

**可选挑战：**

> 处理负的 sbrk() 参数。处理错误用例，如 `sbrk()` 参数过大。验证 `fork()` 和 `exit()` 的运行，即使`sbrk()` 的内存没有为它们分配内存。正确处理栈底部的无效页。确保内核对未分配的用户地址的使用是正常的。例如，如果一个程序传递一个 `sbrk()` 分配的地址给 `read()` 。
>











