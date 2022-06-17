#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string>
#include <unistd.h>
using namespace std;

int main() {
    cout << "Test have" << endl;
    string file_name = "test";
    int fd = open(file_name.c_str(),  O_RDONLY);
    if(fd < 0)
    {
        cout << "open failed." << endl;
        return 0;
    }

    vector<pid_t> pids = {0};
    for(int i = 0; i < 3; i++)
    {
        pids[i] = fork();
        if(pids[i] == 0)
        {
            break;
        }
    }

    for(int i = 0; i < 3; i++)
    {
        if(pids[i] == 0)
        {
            char buf[10];
            read(fd, buf, 10);
            printf("buf:%s \n", buf);
            fflush(stdout);
        }
    }
    close(fd);



    return 0;
}
