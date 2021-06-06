#include "headers.h"

struct processBuffer
{
    int id;
    int arrivalTime;
    int runTime;
    int priority;
    enum processStatus status;
    long mtype;
} processRecv;

void FCFS();
void SJF();
void HPF();
void SRTN();
void RR(int quantum);
int main(int argc, char *argv[])
{
    initClk();

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
void startProcess(Process p)
{
    // to do : fork the Process
    // to do print the log
}
void continueProcess(Process p)
{
    // to do : sigcont(p.pid)
    // to do print the log
}
void stopProcess(Process p)
{
    // to do : sigstop(p.pid)
    // to do print the log
}
void finishProcess(Process p)
{
    // to do : clac waiting , fininsh time..............
    // to do print the log
}
void RR(int quantum)
{
    Queue *q;
    queueConstructor(q);
    int cntQuantum = 0; //conuter for track the quantum of the running Process
    while (1)
    {

        Process p;

        //p = queuePop(q);

        Process arrivalProcess;
        // 1- to do get the Process when it arrival
        //if(getNewProcess)
        //   queuePush(q,arrivalProcess);

        // 2- get the process in the head of the queue
        Process turnProcess = queuePop(q);
        // 3 - to do fork this process and run it

        // to do : if it last process or the algorithm finish we will out from this loop
    }
}
//1
/* p1 p2 p3 */

int main(int argc, char *argv[])
{
    //  initClk();
    FILE *f;
    f = fopen("key", "r");
    key_t key_id = ftok("key", 'a');
    int msgQ = msgget(key_id, 0666 | IPC_CREAT);
    if (msgQ == -1)
    {
        perror("Error in creating message queue");
        return -1;
    }

    /*  struct processHeaders procHeaders;
    procHeaders.mtype = 1;
    int val = msgrcv(msgQ, &procHeaders, sizeof(procHeaders.algorithm) + sizeof(procHeaders.numOfProcesses) + sizeof(procHeaders.processParameter), 0, !IPC_NOWAIT);
    if (val == -1)
        perror("Error in Receiving");
    printf("ChosenAlgorithm: %ld\nnumOfProcesses: %ld\nprocessParam:%ld\n", procHeaders.algorithm, procHeaders.numOfProcesses, procHeaders.processParameter);*/

    while (1)
    {
        processRecv.mtype = 1;
        int val = msgrcv(msgQ, &processRecv, sizeof(processRecv.id) + sizeof(processRecv.arrivalTime) + sizeof(processRecv.runTime) + sizeof(processRecv.priority) + sizeof(processRecv.status), 0, !IPC_NOWAIT);
        if (val == -1)
            perror("Error in Receiving");
        else
            printf("Received Process\n");
        printf("%d %d %d %d\n", processRecv.id, processRecv.arrivalTime, processRecv.runTime, processRecv.priority);
        fflush(stdout);
        if (processRecv.id == procHeaders.numOfProcesses)
            break;
        //TODO: implement the scheduler.
        //TODO: upon termination release the clock resources.
    }
    printf("Terminate msgQ from Scheduler\n");
    msgctl(msgQ, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
}