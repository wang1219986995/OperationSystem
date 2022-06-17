#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
using namespace std;

#define MAX_SIZE 10
static sem_t empty;
static sem_t full;
static sem_t mutex;
const static std::string file_name = "product";
int fd_w;
int fd_r;

bool read_line(int fd, char* buf)
{
    if(fd < 0) return false;
    char tmp;
    int len;
    while((len = read(fd, &tmp, 1)) > 0 && tmp != '\n' )
    {
        *buf++ = tmp;
    }
    return true;
}



void child_func()
{
    cout << "This is child process function, created to consumer." <<  endl;
    fflush(stdout);
    // 理论上，消费者会一直消费
    while(1)
    {
        sem_wait(&mutex);
        sem_wait(&full);

        sem_post(&empty);
        sem_post(&mutex);
    }

}

void parent_func()
{
    cout << "This is parent process function, created to produce." <<  endl;

    // 理论上，生产者会一直生产
    while(1)
    {
        sem_wait(&mutex);
        sem_wait(&empty);
        for(int i = 0; i < MAX_SIZE; i++)
        {
            //write(fd_w, )
        }
        sem_post(&full);
        sem_post(&mutex);
    }
//    sem_wait(&mutex);
//    sem_wait(&empty);
    for(int i = 0; i < MAX_SIZE; i++)
    {
        //write(fd_w, )
    }


}


int main()
{
    cout << "This is unnamede semaphore:" << endl;
    sem_init(&empty, 1, MAX_SIZE);
    sem_init(&full, 1, 0);
    sem_init(&mutex, 1, 1);
    fd_w = open(file_name.c_str(),  O_CREAT | O_WRONLY | O_TRUNC);
    fd_r = open(file_name.c_str(),   O_RDONLY);
    if(fd_w < 0 | fd_r < 0)
    {
        cout << "open failed." << endl;
        return 0;
    }

    pid_t id = fork();
    if(id < 0)  cout << "Fork error" << endl;

    if(id == 0)  child_func();
    else  parent_func();
}