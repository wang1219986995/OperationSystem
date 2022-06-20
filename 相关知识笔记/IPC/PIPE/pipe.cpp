#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
using namespace std;


int main()
{
    int fd[2];
    int res = pipe(fd);
    if(res < 0)
    {
        printf("open pipe failed. \n");
        return 0;
    }

    if(fork() > 0)
    {
        //parent process, prepare to read
        close(fd[1]);
        char buf[32];
        while(read(fd[0], buf, sizeof(buf)))
        {
            printf("read: %s", buf);
        }
    }
    else
    {
        //child process, prepare to write
        close(fd[0]);
        for(int i = 0; i < 10; i++)
        {
            std::string str = "child process message " + to_string(i) + "\n";
            write(fd[1], str.c_str(), str.length());
        }
    }
    return 0;

}