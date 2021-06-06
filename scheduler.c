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
void clearResources(int signum);
void RR(int quantum);
void cleanup(int signum);
struct Queue *q;
struct Process processRecv;
int chosenAlgorithm, paramter = -1, numOfProcesses = 4;
bool lastProcess = 0;
int msgQ;

int countProcess = 0;
int *shmId, shmid; //for the running process
int main(int argc, char *argv[])
{

    signal(SIGINT, cleanup);
    initClk();
    printf("hello i am scheduler...\n");
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

    /*chosenAlgorithm = atoi(argv[1]);
    numOfProcesses = atoi(argv[2]);
    if (chosenAlgorithm == 5)
        paramter = atoi(argv[3]);*/
    // to DO :!!!!!!!! replace it with argc
    chosenAlgorithm = 5;
    if (chosenAlgorithm == 5)
        paramter = 1;

    key_t sharedMemKey = ftok("Makefile", 65);
    shmid = shmget(sharedMemKey, 4000, 0666 | IPC_CREAT); // crete shared
    if (shmid == -1)
    {
        perror("Error in creating message queue");
        return -1;
    }
    shmId = (int *)shmat(shmid, (void *)0, 0);

    RR(paramter);
    printf("Terminate msgQ from Scheduler\n");
    msgctl(msgQ, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
}

int startProcess(Process p)
{

    // to do : fork the Process
    // to do print the log
    p.pid = fork();
    if (p.pid == 0)
    {
        system("gcc process.c -o process.out");
        execl("./process.out", "process", NULL);
    }
    int waitingTime = getClk() - p.arrivalTime;
    printf("at time %d process %d started arrive time %d running time %d remning time %d waiting time %d", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, waitingTime);
    return p.id;
}
void continueProcess(Process p)
{
    // to do : sigcont(p.pid)
    // to do print the log
    kill(p.pid, SIGCONT);
    int waitingTime = getClk() - p.arrivalTime;
    printf("at time %d process %d continued arrive time %d running time %d remning time %d waiting time %d", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, waitingTime);
}
void stopProcess(Process p)
{
    // to do : sigstop(p.pid)
    // to do print the log
    *shmId = -1;
    kill(p.pid, SIGSTOP);
    int waitingTime = getClk() - p.arrivalTime;
    printf("at time %d process %d stoped arrive time %d running time %d remning time %d waiting time %d", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, waitingTime);
}
void finishProcess(Process p)
{
    // to do : clac waiting , fininsh time..............
    // to do print the log
    int waitingTime = getClk() - p.arrivalTime;
    printf("at time %d process %d finished arrive time %d running time %d remning time %d waiting time %d", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, waitingTime);
}
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
                countProcess++;
                printf("Received Process\n");
                printf("%d %d %d %d RecvTime:%d\n", processRecv.id, processRecv.arrivalTime, processRecv.runTime, processRecv.priority, processRecv.sendTime);
                //  queuePush(q, processRecv);
                if (processRecv.id == numOfProcesses)
                    break;
            }
            else
            {
                printf("Received a fake Process\n");
            }
        }
    }
    return;
}
void RR(int quantum)
{
    printf("Schuder: hello i started RR...\n");

    q = malloc(sizeof(Queue));
    queueConstructor(q);
    int cntQuantum = quantum; //conuter for track the quantum of the running Process
    struct Process running;
    *shmId = -1;
    while (1)
    {
        // struct Process p;
        printf("Schuder: hello i am in RR and time %d...\n", getClk());

        if (countProcess < numOfProcesses)
            getArrivalProcessAndPushIt();

        if (*shmId == 0) //running process is finish
        {
            finishProcess(running);
            cntQuantum = quantum;
        }
        if (!queueIsEmpty(q) && (cntQuantum == 0 || *shmId <= 0))
        {
            if (*shmId != -1) //no finish yet
            {
                running.remningTime = *shmId;
                queuePush(q, running);
                stopProcess(running);
            }
            struct Process front = queuePop(q);
            *shmId = front.remningTime + 1;
            running = front;
            if (front.remningTime < front.runTime) // it's contiue
                continueProcess(running);
            else //first time to run
                running.pid = startProcess(running);
        }
        cntQuantum--;
        sleep(1);
        countProcess++;
        // to do : if it last process or the algorithm finish we will out from this loop done
        if (countProcess == numOfProcesses && queueIsEmpty(q) && *shmId <= 0)
            break;
        // }

        printf("Schuder: hello i finished RR...\n");
    }
}
void cleanup(int signum)
{
    printf("Terminate shared memory from scheduler\n");
    shmctl(shmid, IPC_RMID, NULL);
    kill(getpid(), SIGKILL);
}