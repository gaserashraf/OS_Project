#include "headers.h"

/* Modify this file as needed*/
int *shmId;
int main(int agrc, char *argv[])
{
    initClk();
    //printf("hello from main process.c\n");
    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    key_t sharedMemKey = ftok("Makefile", 65);
    int shmid = shmget(sharedMemKey, 4000, 0666 | IPC_CREAT); // crete shared
    if (shmid == -1)
    {
        perror("Error in creating message queue");
        return -1;
    }
    shmId = (int *)shmat(shmid, (void *)0, 0);
    int prevTime = -1;
    while (*shmId > 0)
    {
        // remainingtime = ??;
        while (prevTime == getClk())
            ;
        if (prevTime != getClk())
        {
            //   printf("hello iam process.c i will decrese 1 sec from shmid %d\n", *shmId);
            (*shmId)--;
            prevTime = getClk();
        }
    }

    destroyClk(false);

    return 0;
}
