# 系统调用

## 1 实验内容

此次实验的基本内容是：在 Linux 0.11 上添加两个系统调用，并编写两个简单的应用程序测试它们。

（1）`iam()`

第一个系统调用是 iam()，其原型为：

```c
int iam(const char * name);
```

完成的功能是将字符串参数 `name` 的内容拷贝到内核中保存下来。要求 `name` 的长度不能超过 23 个字符。返回值是拷贝的字符数。如果 `name` 的字符个数超过了 23，则返回 “-1”，并置 errno 为 EINVAL。

在 `kernal/who.c` 中实现此系统调用。

（2）`whoami()`

第二个系统调用是 whoami()，其原型为：

```c
int whoami(char* name, unsigned int size);
```

它将内核中由 `iam()` 保存的名字拷贝到 name 指向的用户地址空间中，同时确保不会对 `name` 越界访存（`name` 的大小由 `size` 说明）。返回值是拷贝的字符数。如果 `size` 小于需要的空间，则返回“-1”，并置 errno 为 EINVAL。

也是在 `kernal/who.c` 中实现。

（3）测试程序

运行添加过新系统调用的 Linux 0.11，在其环境下编写两个测试程序 iam.c 和 whoami.c。最终的运行结果是：

```
$ ./iam lizhijun
$ ./whoami
lizhijun
```



**问题：**

在实验报告中回答如下问题：

- 从 Linux 0.11 现在的机制看，它的系统调用最多能传递几个参数？你能想出办法来扩大这个限制吗？
- 用文字简要描述向 Linux 0.11 添加一个系统调用 foo() 的步骤。






## 2 实验步骤



include/unistd.h  中增加如下定义：

```c
#define __NR_setregid	71
#define __NR_iam	72
#define __NR_whoami	73
```

修改 kernel/system_call.s:

```asm
nr_system_calls = 74 	;此处又72修改为74
```

在 include/linux/sys.h 中增加如下内容：

```c
... ...
extern int sys_setregid();
extern int sys_iam();		// 增加 iam() 的系统调用
extern int sys_whoami();	// 增加 whoami() 的系统调用

fn_ptr sys_call_table[] = { sys_setup, sys_exit,
                   ... ...
, sys_iam, sys_whoami};		// 在列表中增加sys_iam, sys_whoami
```



在 kernel/ 目录下新建who.c 文件，并增加如下代码：

```c
#include <asm/segment.h>
#include <errno.h>
#include <string.h>

#define UNAMESIZE 32
static char username[UNAMESIZE] = {0};

int sys_iam(const char* name)
{
    char str[UNAMESIZE + 1];
    int i = 0;
    while(i <= UNAMESIZE)
    {
        str[i] = get_fs_byte(name + i);
        if(str[i++] == 0)
            break;
    }

    if(i > UNAMESIZE)
    {
        errno = EINVAL;
        return -1;
    }
    strcpy(username, str);
    return i;
}

int sys_whoami(char* name, unsigned int size)
{
    int length = strlen(username);
    if(size < length)
    {
        errno = EINVAL;
        return -1;
    }
    int i;
    for(i = 0; i < length; i++)
        put_fs_byte(username[i], name + i);
    return length;
}
```



修改Makefile：

1. 第一处

```
OBJS  = sched.o system_call.o traps.o asm.o fork.o \
        panic.o printk.o vsprintf.o sys.o exit.o \
        signal.o mktime.o


```

改为：

```
OBJS  = sched.o system_call.o traps.o asm.o fork.o \
        panic.o printk.o vsprintf.o sys.o exit.o \
        signal.o mktime.o who.o


```

添加了 `who.o`。

（2）第二处

```
### Dependencies:
exit.s exit.o: exit.c ../include/errno.h ../include/signal.h \
  ../include/sys/types.h ../include/sys/wait.h ../include/linux/sched.h \
  ../include/linux/head.h ../include/linux/fs.h ../include/linux/mm.h \
  ../include/linux/kernel.h ../include/linux/tty.h ../include/termios.h \
  ../include/asm/segment.h


```

改为：

```
### Dependencies:
who.s who.o: who.c ../include/linux/kernel.h ../include/unistd.h
exit.s exit.o: exit.c ../include/errno.h ../include/signal.h \
  ../include/sys/types.h ../include/sys/wait.h ../include/linux/sched.h \
  ../include/linux/head.h ../include/linux/fs.h ../include/linux/mm.h \
  ../include/linux/kernel.h ../include/linux/tty.h ../include/termios.h \
  ../include/asm/segment.h


```

添加了 `who.s who.o: who.c ../include/linux/kernel.h ../include/unistd.h`。







编写测试程序 `whoami.c` 和 `iam.c` :

```c
// whoami.c
#define __LIBRARY__
#include <unistd.h> 
#include <errno.h>
#include <asm/segment.h> 
#include <linux/kernel.h>
#include <stdio.h>
   
#define __NR_whoami 73
_syscall2(int, whoami,char *,name,unsigned int,size);
   
int main(int argc, char *argv[])
{
    char username[64] = {0};
    /*调用系统调用whoami()*/
    whoami(username, 24);
    printf("%s\n", username);
    return 0;
}

// iam.c
#define __LIBRARY__
#include <unistd.h> 
#include <errno.h>
#include <asm/segment.h> 
#include <linux/kernel.h>
#define __NR_iam 72
_syscall1(int, iam, const char*, name);
   
int main(int argc, char *argv[])
{
    /*调用系统调用iam()*/
    iam(argv[1]);
    return 0;
}
```



将 `whoami.c` 和 `iam.c`  放到hdc磁盘中，在linux 0.11 上编译运行：

```shell
$ sudo ./mount-hdc 
$ cp iam.c whoami.c hdc/usr/root
```

编译运行：

```shell
[/usr/root]# gcc -o iam iam.c
[/usr/root]# gcc -o whoami whoami.c
[/usr/root]# ./iam wcf
[/usr/root]# ./whoami
```