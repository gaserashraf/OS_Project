#include "headers.h"

/*struct processBuffer
{
    int id;
    int arrivalTime;
    int runTime;
    int priority;
    enum processStatus status;
    Process p;
    long mtype;
    bool haveProcess;
}*/

void FCFS();
void SJF();
void HPF();
void SRTN();
void getArrivalProcessAndPushIt();
void RR(int quantum);

struct Queue *q;
struct Process processRecv;
int chosenAlgorithm, paramter = -1, numOfProcesses = -1;
bool lastProcess = 0;
int msgQ;

int *shmId; //for the running process
int main(int argc, char *argv[])
{
    //  initClk();
    FILE *f;
    f = fopen("key", "r");
    key_t key_id = ftok("key", 'a');
    msgQ = msgget(key_id, 0666 | IPC_CREAT);
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
    chosenAlgorithm = atoi(argv[1]);
    numOfProcesses = atoi(argv[2]);
    if (chosenAlgorithm == 5)
        paramter = atoi(argv[3]);

    // key_t sharedMemKey = ftok("Makefile", 65);
    // shmId = shmget(sharedMemKey, 4000, 0666 | IPC_CREAT); // crete shared
    getArrivalProcessAndPushIt();
    printf("Terminate msgQ from Scheduler\n");
    msgctl(msgQ, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
}
/*int startProcess(Process p)
{
    
    // to do : fork the Process
    // to do print the log
    p.pid = fork();
    if(p.pid==0)
    {
        system("gcc process.c -o process");
        execl("./process")
    }
    printf("at time %d process %d started arrive time %d running time %d remning time %d waiting time %d",getClk(),p.arrivalTime, p.runTime,p.remningTime,p.waitingTime);
    return p.id;
}
void continueProcess(Process p)
{
    // to do : sigcont(p.pid)
    // to do print the log
    kill(p.pid,SIGCONT);
    printf("at time %d process %d continued arrive time %d running time %d remning time %d waiting time %d",getClk(),p.arrivalTime, p.runTime,p.remningTime,getClk()-p.arrivalTime);
}
void stopProcess(Process p)
{
    // to do : sigstop(p.pid)
    // to do print the log
    *shmId=-1;
     kill(p.pid,SIGSTOP);
    printf("at time %d process %d stoped arrive time %d running time %d remning time %d waiting time %d",getClk(),p.arrivalTime, p.runTime,p.remningTime,getClk()-p.arrivalTime);

}
void finishProcess(Process p)
{
    // to do : clac waiting , fininsh time..............
    // to do print the log
    printf("at time %d process %d finished arrive time %d running time %d remning time %d waiting time %d",getClk(),p.arrivalTime, p.runTime,p.remningTime,getClk()-p.arrivalTime);
}*/
void getArrivalProcessAndPushIt()
{
    while (1)
    {
        processRecv.valid = 0; //clear prev recv mess
        processRecv.mtype = 1;
        int val = msgrcv(msgQ, &processRecv, 100 * sizeof(processRecv), 0, !IPC_NOWAIT);
        if (val == -1)
        {
            perror("Error in Receiving");
            break;
        }
        else
        {
            if (processRecv.valid == 1)
            {
                printf("Received Process\n");
                printf("%d %d %d %d RecvTime:%d\n", processRecv.id, processRecv.arrivalTime, processRecv.runTime, processRecv.priority, processRecv.sendTime);
                //  queuePush(q, processRecv);
                if (processRecv.id == numOfProcesses)
                    break;
            }
        }
    }
    return;
}
/*void RR(int quantum)
{
   
    queueConstructor(q);
    int cntQuantum = quantum; //conuter for track the quantum of the running Process
    struct Process running;
    *shmId=-1;
    while (1)
    {

        Process p;

        //p = queuePop(q);
        // 1- to do get the Process when it arrival
        getArrivalProcessAndPushIt();
       
        if(*shmId==0)//running process is finish
        {
            finishProcess(running);
            cntQuantum=quantum;
        }
        if(!queueIsEmpty(q)&&(cntQuantum==0||*shmId<=0))
        {
            if(*shmId!=-1)//no finish yet
            {
                running.remningTime=*shmId;
                queuePush(running);
                stopProcess(running);
            }
            struct Process front = queuePop(q);
            *shmId.remningTime = front.remningTime+1;
            running=front;
            if(front.remningTime < front.runTime) // it's contiue
                continueProcess(running);
            else //first time to run
                running.pid = startProcess(running);
        }
        // to do : if it last process or the algorithm finish we will out from this loop
    }
}
//1*/
/* p1 p2 p3 */