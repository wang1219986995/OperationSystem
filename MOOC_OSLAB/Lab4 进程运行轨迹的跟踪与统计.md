# Lab4 进程运行轨迹的跟踪与统计



本次实验包括如下内容：

- 基于模板 `process.c` 编写多进程的样本程序，实现如下功能： 

  所有子进程都并行运行，每个子进程的实际运行时间一般不超过 30 秒； 

  父进程向标准输出打印所有子进程的 id，并在所有子进程都退出后才退出；

- 在 `Linux0.11` 上实现进程运行轨迹的跟踪。

  基本任务是在内核中维护一个日志文件 `/var/process.log`，把从操作系统启动到系统关机过程中所有进程的运行轨迹都记录在这一 log 文件中。

- 在修改过的 0.11 上运行样本程序，通过分析 log 文件，统计该程序建立的所有进程的等待时间、完成时间（周转时间）和运行时间，然后计算平均等待时间，平均完成时间和吞吐量。可以自己编写统计程序，也可以使用 python 脚本程序—— `stat_log.py`（在 `/home/teacher/` 目录下） ——进行统计。

- 修改 0.11 进程调度的时间片，然后再运行同样的样本程序，统计同样的时间数据，和原有的情况对比，体会不同时间片带来的差异。

  ​

完成实验后，在实验报告中回答如下问题：

- 结合自己的体会，谈谈从程序设计者的角度看，单进程编程和多进程编程最大的区别是什么？
- 你是如何修改时间片的？仅针对样本程序建立的进程，在修改时间片前后，log 文件的统计结果（不包括 Graphic）都是什么样？结合你的修改分析一下为什么会这样变化，或者为什么没变化？



## 1 process.c 实现

创建10个进程，代码实现如下：

```c++
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <sys/times.h>
#include <vector>
#include <sys/wait.h>

using namespace std;
#define HZ	100

void cpuio_bound(int last, int cpu_time, int io_time);

int main(int argc, char * argv[])
{
    printf("Father process begining: \n");
    int pro_number = 10;
    int pid_vec[10];
    int i;
    for( i = 0; i < pro_number; i++)
    {
        pid_vec[i] = fork();
        if(pid_vec[i] == 0)
        {
            int single_time = i + 1;
            cpuio_bound(2 * single_time, single_time, single_time);
            break;
        }
        else if(pid_vec[i] == -1)
        {
            printf("Child process error, number = %d \n", i);
            exit(1);
        }
    }

    if(i >= pro_number)
    {
        for(int j = 0; j < pro_number; j++)
        {
            int status;
            waitpid(pid_vec[j], &status, 0);
            printf("Father Process: %d child process has terminated.\n", pid_vec[j]);
        }
    }
    return 0;
}

/*
 * 此函数按照参数占用CPU和I/O时间
 * last: 函数实际占用CPU和I/O的总时间，不含在就绪队列中的时间，>=0是必须的
 * cpu_time: 一次连续占用CPU的时间，>=0是必须的
 * io_time: 一次I/O消耗的时间，>=0是必须的
 * 如果last > cpu_time + io_time，则往复多次占用CPU和I/O
 * 所有时间的单位为秒
 */
void cpuio_bound(int last, int cpu_time, int io_time)
{
    struct tms start_time, current_time;
    clock_t utime, stime;
    int sleep_time;

    while (last > 0)
    {
        /* CPU Burst */
        times(&start_time);
        /* 其实只有t.tms_utime才是真正的CPU时间。但我们是在模拟一个
         * 只在用户状态运行的CPU大户，就像“for(;;);”。所以把t.tms_stime
         * 加上很合理。*/
        do
        {
            times(&current_time);
            utime = current_time.tms_utime - start_time.tms_utime;
            stime = current_time.tms_stime - start_time.tms_stime;
        } while ( ( (utime + stime) / HZ )  < cpu_time );
        last -= cpu_time;

        if (last <= 0 )
            break;

        /* IO Burst */
        /* 用sleep(1)模拟1秒钟的I/O操作 */
        sleep_time=0;
        while (sleep_time < io_time)
        {
            sleep(1);
            sleep_time++;
        }
        last -= sleep_time;
    }
}
```

执行结果如下所示：

```shell
ubuntu@VM-20-12-ubuntu:~/tmp/tmp.1LX1ijg3vV$ ./main 
Father process begining: 
Father Process: 22442 child process has terminated.
Father Process: 22443 child process has terminated.
Father Process: 22444 child process has terminated.
Father Process: 22445 child process has terminated.
Father Process: 22446 child process has terminated.
Father Process: 22447 child process has terminated.
Father Process: 22448 child process has terminated.
Father Process: 22449 child process has terminated.
Father Process: 22450 child process has terminated.
Father Process: 22451 child process has terminated.
ubuntu@VM-20-12-ubuntu:~/tmp/tmp.1LX1ijg3vV$ 
```



## 2 进程运行轨迹输出至日志文件

首先修改 init/main.c 文件：



```c

void init(void)
{
	int pid,i;
	// 将以下几行代码从此处删除，移动到 main() 函数中
	setup((void *) &drive_info);
	(void) open("/dev/tty0",O_RDWR,0);
	(void) dup(0);
	(void) dup(0);                
  
  
  void main(void)	
  {
    ... ...
    move_to_user_mode();
    // 上面删掉的几行放到这里
	setup((void *) &drive_info);
	(void) open("/dev/tty0",O_RDWR,0);
	(void) dup(0);
	(void) dup(0);
    // 增加打开 process.log 文件
    (void) open("/var/process.log",O_CREAT|O_TRUNC|O_WRONLY,0666);
    ... ... 
  }
  
```



在内核状态下，write() 功能失效，其原理等同于《系统调用》实验中不能在内核状态调用 `printf()`，只能调用 `printk()`。编写可在内核调用的 `write()` 的难度较大，所以这里直接给出源码。它主要参考了 `printk()` 和 `sys_write()` 而写成的，将以下代码放入 `kernel/printk.c` 文件中：





```c
#include <stdarg.h>
#include <stddef.h>

#include <linux/kernel.h>
#include "linux/sched.h"
#include "sys/stat.h"

static char buf[1024];
extern int vsprintf(char * buf, const char * fmt, va_list args);
int printk(const char *fmt, ...)
{
	... ...
}

static char logbuf[1024];
int fprintk(int fd, const char *fmt, ...)
{
    va_list args;
    int count;
    struct file * file;
    struct m_inode * inode;

    va_start(args, fmt);
    count=vsprintf(logbuf, fmt, args);
    va_end(args);
/* 如果输出到stdout或stderr，直接调用sys_write即可 */
    if (fd < 3)
    {
        __asm__("push %%fs\n\t"
            "push %%ds\n\t"
            "pop %%fs\n\t"
            "pushl %0\n\t"
        /* 注意对于Windows环境来说，是_logbuf,下同 */
            "pushl $logbuf\n\t"
            "pushl %1\n\t"
        /* 注意对于Windows环境来说，是_sys_write,下同 */
            "call sys_write\n\t"
            "addl $8,%%esp\n\t"
            "popl %0\n\t"
            "pop %%fs"
            ::"r" (count),"r" (fd):"ax","cx","dx");
    }
    else
/* 假定>=3的描述符都与文件关联。事实上，还存在很多其它情况，这里并没有考虑。*/
    {
    /* 从进程0的文件描述符表中得到文件句柄 */
        if (!(file=task[0]->filp[fd]))
            return 0;
        inode=file->f_inode;

        __asm__("push %%fs\n\t"
            "push %%ds\n\t"
            "pop %%fs\n\t"
            "pushl %0\n\t"
            "pushl $logbuf\n\t"
            "pushl %1\n\t"
            "pushl %2\n\t"
            "call file_write\n\t"
            "addl $12,%%esp\n\t"
            "popl %0\n\t"
            "pop %%fs"
            ::"r" (count),"r" (file),"r" (inode):"ax","cx","dx");
    }
    return count;
}
```

使用方法：

```c
// 向stdout打印正在运行的进程的ID
fprintk(1, "The ID of running process is %ld", current->pid);

// 向log文件输出跟踪进程运行轨迹
fprintk(3, "%ld\t%c\t%ld\n", current->pid, 'R', jiffies);
```



Log文件中要输出进程的 新建(N)、就绪(J)、运行(R)、阻塞(W) 和退出(E) 5种状态，其中对于某一进程而言，新建(N)、退出(E) 进输出一次。









