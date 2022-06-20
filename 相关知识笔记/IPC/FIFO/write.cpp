#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
using namespace std;


std::string fifo_name = "/tmp/fifo";
int main()
{
    int fd = open(fifo_name.c_str(), O_WRONLY);
    if(fd < 0)
    {
        printf("open mkfifo failed.\n");
        return 0;
    }

    string str;
    while(cin >> str)
    {
        write(fd, str.c_str(), str.length());
    }
    return 0;

}