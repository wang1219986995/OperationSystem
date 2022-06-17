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


void child_func()
{
    cout << "This is child process function, created to consumer." <<  endl;

}

void parent_func()
{
    cout << "This is parent process function, created to produce." <<  endl;
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