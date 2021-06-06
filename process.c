#include "headers.h"

/* Modify this file as needed*/
int *remainingtime;
int *shmId;
int main(int agrc, char *argv[])
{
    initClk();

    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    key_t sharedMemKey = ftok("Makefile",65);
    shmId = shmget(sharedMemKey, 4000, 0666 | IPC_CREAT); // crete shared
    *remainingtime=*shmId;
    int prevTime=-1;
    while (*remainingtime > 0)
    {
        // remainingtime = ??;
        while(prevTime==getClk());
        if(prevTime!=getClk())
        {
            (*remainingtime)--;
            prevTime=getClk();
        }
    }

    destroyClk(false);

    return 0;
}
