#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <string>
using namespace std;

std::string fifo_name = "/tmp/fifo";
int main()
{
    remove(fifo_name.c_str());
    int fd = mkfifo(fifo_name.c_str(), 0777);
    if(fd < 0)
    {
        printf("mkfifo failed.\n");
        return 0;
    }

    fd = open(fifo_name.c_str(), O_RDONLY);
    char buf[32];
    while(read(fd, buf, sizeof(buf)) > 0)
    {
        printf("read: %s \n", buf);
    }
    close(fd);
    return 0;
}