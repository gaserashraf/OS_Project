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

/*
// call it when looping over all waitingList items

//////////////////////////////
bool firstFitAllocate(Process *p);
bool FFDeallocate(Process *p);




/////////////////////////////

*/

void FCFS();
void SJF();
void HPF();
void SRTN();
void getArrivalProcessAndPushIt();
void clearResources(int signum);
void RR(int quantum);
void cleanup(int signum);
bool nextfit();
bool nextfitallocatingmem(Process *p);
void nextfitdeallocatingmem(Process *p);
bool firstFitAllocate(Process *p);
bool firstFitDeallocate(Process *p);
bool firstfit();
struct linkedlist *waitingListFirstFit = NULL;
int memoryFirstFit[1024]; // 0=>free & 1=>used
struct Queue *q = NULL;
struct priorityQueue *pq = NULL;
struct linkedlist *waitList = NULL; //emad
struct Process processRecv;
int chosenAlgorithm, paramter = -1, numOfProcesses = 5, chosenPolicy = -1;
bool lastProcess = 0;
int msgQ;
bool flag = 0;        //emad
bool memory[1024];    //the memory         emad
int pointertomem = 0; //pointer to the memory used in next fit algorithm   emad
bool allocate = 0;    //flag to check if there is enough memory       emad
bool nomem = 0;       //flag to check if there is no enough memory        emad

bool isProcessRunNow = 0;

int countProcess = 0;
int *shmId, shmid; //for the running process

FILE *schedulerLog;
FILE *MemoryLog;

int main(int argc, char *argv[])
{
    for (int i = 0; i < 1024; i++)
    {
        memory[i] = 0;
    }
    waitList = (linkedlist *)malloc(sizeof(linkedlist)); //emad
    linkedlistConstructor(waitList);                     //emad

    waitingListFirstFit = (linkedlist *)malloc(sizeof(linkedlist)); //emad
    linkedlistConstructor(waitingListFirstFit);                     //emad

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

    schedulerLog = fopen("schedular.log", "w");
    fprintf(schedulerLog, "#at time x process y started arrive time w running time z remning time y waiting time k\n");
    MemoryLog = fopen("memory.log", "w");
    fprintf(MemoryLog, "at time x allocated y bytes for Processs z from i to j\n");

    chosenAlgorithm = atoi(argv[1]);
    numOfProcesses = atoi(argv[2]);
    if (chosenAlgorithm == 5)
    {
        paramter = atoi(argv[3]);
        chosenPolicy = atoi(argv[4]);
    }
    else
        chosenPolicy = atoi(argv[3]);

    key_t sharedMemKey = ftok("Makefile", 65);
    shmid = shmget(sharedMemKey, 4000, 0666 | IPC_CREAT); // crete shared
    if (shmid == -1)
    {
        perror("Error in creating message queue");
        return -1;
    }
    shmId = (int *)shmat(shmid, (void *)0, 0);
    switch (chosenAlgorithm)
    {
    case 1:
        FCFS();
        break;
    case 2:
        SJF();
        break;
    case 3:
        HPF();
        break;
    case 4:
        SRTN();
        break;
    case 5:
        RR(paramter);
        break;
    default:
        break;
    }
    printf("Terminate msgQ from Scheduler\n");
    fclose(MemoryLog);
    fclose(schedulerLog);
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
        //printf("hello form fork\n");
        system("gcc process.c -o process.out");
        execl("./process.out", "process", NULL);
    }

    p.waitingTime = getClk() - p.arrivalTime;
    printf("at time %d process %d started arrive time %d running time %d remning time %d waiting time %d\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, p.waitingTime);
    fprintf(schedulerLog, "at time %d process %d started arrive time %d running time %d remning time %d waiting time %d\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, p.waitingTime);

    //printf("at time %d process %d started arrive time %d running time %d remning time %d waiting time %d\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, waitingTime);
    return p.pid;
}
void continueProcess(Process p)
{
    // to do : sigcont(p.pid)
    // to do print the log
    kill(p.pid, SIGCONT);
    //  p.waitingTime += getClk() - p.stopTime;
    printf("at time %d process %d continued arrive time %d running time %d remning time %d waiting time %d\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, p.waitingTime);
    fprintf(schedulerLog, "at time %d process %d continued arrive time %d running time %d remning time %d waiting time %d\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, p.waitingTime);
    //printf("at time %d process %d continued arrive time %d running time %d remning time %d waiting time %d\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, waitingTime);
}
void stopProcess(Process p)
{
    // to do : sigstop(p.pid)
    // to do print the log
    kill(p.pid, SIGSTOP);
    p.stopTime = getClk();
    fprintf(schedulerLog, "at time %d process %d stoped arrive time %d running time %d remning time %d waiting time %d\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, p.waitingTime);
    //printf("at time %d process %d stop arrive time %d running time %d remning time %d waiting time %d\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, waitingTime);
}
void finishProcess(Process p)
{
    // to do : clac waiting , fininsh time..............
    // to do print the log
    double WTA = (getClk() - p.arrivalTime) * 1.0 / p.runTime;
    p.remningTime = 0;
    *shmId = -1;
    switch (chosenPolicy)
    {
    case 1:
        firstFitDeallocate(&p);
        break;
    case 2:
        nextfitdeallocatingmem(&p);
        break;
    default:
        break;
    }
    fprintf(MemoryLog, "at time %d Deallocated %d bytes for process %d from %d to %d\n", getClk(), p.memSize, p.id, p.startIndex, p.endIndex);
    flag = 0;
    node *temp = waitList->head;
    while (temp != NULL)
    {
        if (nextfitallocatingmem(&temp->data))
        {
            printf("FCFS : i will pick process now\n");
            *shmId = temp->data.remningTime + 1;
            flag = 1;
            temp->data.waitingTime = getClk() - temp->data.arrivalTime;
            fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), temp->data.memSize, temp->data.id, temp->data.startIndex, temp->data.endIndex);
            temp->data.pid = startProcess(temp->data);
        }
    }
    if (flag)
    {
        isProcessRunNow = 1;
    }
    fprintf(schedulerLog, "at time %d process %d finished arrive time %d running time %d remning time %d waiting time %d TA %d WTA %.2f\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, p.waitingTime, getClk() - p.arrivalTime, WTA);
    //printf("at time %d process %d finish arrive time %d running time %d remning time %d waiting time %d\n", getClk(), p.id, p.arrivalTime, p.runTime, p.remningTime, waitingTime);
}

void RR(int quantum)
{
    printf("Schuder: hello i started RR...\n");
    q = (Queue *)malloc(sizeof(Queue));
    queueConstructor(q);
    int cntQuantum = quantum; //conuter for track the quantum of the running Process
    struct Process running;
    *shmId = 0;
    isProcessRunNow = 0;
    int timeWillEndHisQuantum = 0;
    while (1)
    {
        /*if (isProcessRunNow && *shmId <= 0) //running process is finish
        {
            printf("Sche : finish process\n");
            running.remningTime = 0;
            finishProcess(running);
            isProcessRunNow = 0;
        }*/
        if (isProcessRunNow && timeWillEndHisQuantum <= getClk()) //no finish yet(stop it and push it pack to queue)
        {
            printf("Sche : stop process\n");
            running.remningTime -= quantum;

            if (running.remningTime <= 0)
            {
                running.remningTime = 0;
                finishProcess(running);
            }
            else
            {
                queuePush(q, running);
                stopProcess(running);
            }
            isProcessRunNow = 0;
        }

        switch (chosenPolicy)
        {
        case 1:
            if (firstfit())
            {
                printf("kaaaaaaaaaaaaak\n");
                isProcessRunNow = 1;
            }
            if (!queueIsEmpty(q) && !isProcessRunNow) //pick front of the q
            {
                printf("Sche : pick process\n");
                running = queuePop(q);
                *shmId = running.remningTime + 1;
                timeWillEndHisQuantum = min(getClk() + quantum, getClk() + running.remningTime);
                isProcessRunNow = 1;
                if (running.remningTime < running.runTime) // it's contiue
                {
                    printf("Sche : continue process\n");
                    continueProcess(running);
                }
                else //first time to run
                {
                    printf("Sche : start process will stop at %d\n", timeWillEndHisQuantum);
                    if (firstFitAllocate(&running))
                    {
                        isProcessRunNow = 1;
                        running.waitingTime = getClk() - running.arrivalTime;
                        fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                        running.pid = startProcess(running);
                    }
                }
            }
            break;
        case 2:
            if (nextfit())
                isProcessRunNow = 1;
            if (!queueIsEmpty(q) && !isProcessRunNow) //pick front of the q
            {
                printf("Sche : pick process\n");
                running = queuePop(q);
                *shmId = running.remningTime + 1;
                timeWillEndHisQuantum = min(getClk() + quantum, getClk() + running.remningTime);
                isProcessRunNow = 1;
                if (running.remningTime < running.runTime) // it's contiue
                {
                    printf("Sche : continue process\n");
                    continueProcess(running);
                }
                else //first time to run
                {
                    printf("Sche : start process will stop at %d\n", timeWillEndHisQuantum);
                    if (firstFitAllocate(&running))
                    {
                        isProcessRunNow = 1;
                        running.waitingTime = getClk() - running.arrivalTime;
                        fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                        running.pid = startProcess(running);
                    }
                }
            }
            break;
        default:
            break;
        }

        // struct Process p;
        /* revivce process*/
        processRecv.valid = 0; //clear prev recv mess
        processRecv.mtype = 1;
        if (countProcess < numOfProcesses)
        {
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
                    processRecv.remningTime = processRecv.runTime;
                    printf("%d %d %d %d RecvTime:%d\n", processRecv.id, processRecv.arrivalTime, processRecv.runTime, processRecv.priority, processRecv.sendTime);
                    queuePush(q, processRecv);
                    printQueue(q);
                    //if (processRecv.id == numOfProcesses)
                    //break;
                }
                else
                {
                    printf("Received a fake Process\n");
                    continue;
                }
            }
        }

        // to do : if it last process or the algorithm finish we will out from this loop , done
        if (countProcess == numOfProcesses && queueIsEmpty(q) && !isProcessRunNow)
            break;
    }
    printf("Schuder: hello i finished RR...\n");
}
void FCFS()
{
    printf("Schuder: hello i started FCFS...\n");
    q = (Queue *)malloc(sizeof(Queue));
    queueConstructor(q);

    Process running;
    *shmId = -1;

    isProcessRunNow = 0;
    while (1)
    {
        // struct Process p;
        /* revivce process*/
        //printf("FCFS %d, sharmem %d\n",getClk(),*shmId);
        processRecv.valid = 0; //clear prev recv mess
        processRecv.mtype = 1;
        if (countProcess < numOfProcesses)
        {
            printf("FCFS : i will receive now\n");
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
                    processRecv.remningTime = processRecv.runTime;
                    printf("%d %d %d %d RecvTime:%d\n", processRecv.id, processRecv.arrivalTime, processRecv.runTime, processRecv.priority, processRecv.sendTime);
                    queuePush(q, processRecv);
                    printQueue(q);
                    //if (processRecv.id == numOfProcesses)
                    //break;
                }
                else
                {
                    printf("Received a fake Process\n");
                    //continue;
                }
            }
        }

        // printf("shmId= %d\n", *shmId);
        if (*shmId <= 0 && isProcessRunNow) //running process is finish
        {
            printf("FCFS : i will end process now\n");
            isProcessRunNow = 0;
            finishProcess(running);
        }
        // to do pick a new process
        switch (chosenPolicy)
        {
        case 1:
            if (firstfit())
            {
                isProcessRunNow = 1;
            }
            if (!queueIsEmpty(q) && !isProcessRunNow)
            {
                printf("FCFS : i will pick process now\n");
                struct Process front = queuePop(q);
                *shmId = front.remningTime + 1;
                running = front;

                if (firstFitAllocate(&running))
                {
                    isProcessRunNow = 1;
                    running.waitingTime = getClk() - running.arrivalTime;
                    fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                    running.pid = startProcess(running);
                }
            }
            break;
        case 2:
            if (nextfit())
                isProcessRunNow = 1;
            if (!queueIsEmpty(q) && !isProcessRunNow)
            {
                printf("FCFS : i will pick process now\n");
                struct Process front = queuePop(q);
                *shmId = front.remningTime + 1;
                running = front;

                if (nextfitallocatingmem(&running))
                {
                    isProcessRunNow = 1;
                    running.waitingTime = getClk() - running.arrivalTime;
                    fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                    running.pid = startProcess(running);
                }
            }
            break;
        default:
            break;
        }

        // to do : if it last process or the algorithm finish we will out from this loop done
        if (countProcess == numOfProcesses && queueIsEmpty(q) && !isProcessRunNow)
            break;
    }
    printf("Schudler: hello i finished FCFS...\n");
}
void SJF() //smallest running time first
{
    printf("Schuder: hello i started SJF...\n");
    pq = (priorityQueue *)malloc(sizeof(priorityQueue));
    priorityQueueConstructor(pq);
    Process running;
    *shmId = -1;
    isProcessRunNow = 0;
    while (1)
    {
        // struct Process p;
        /* revivce process*/
        //printf("FCFS %d, sharmem %d\n",getClk(),*shmId);
        processRecv.valid = 0; //clear prev recv mess
        processRecv.mtype = 1;
        if (countProcess < numOfProcesses)
        {
            printf("SJF : i will receive now\n");
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
                    processRecv.remningTime = processRecv.runTime;
                    printf("%d %d %d %d RecvTime:%d\n", processRecv.id, processRecv.arrivalTime, processRecv.runTime, processRecv.priority, processRecv.sendTime);
                    priorityQueuePush(pq, processRecv, processRecv.runTime);
                    printPriorityQueue(pq);
                    //if (processRecv.id == numOfProcesses)
                    //break;
                }
                else
                {
                    printf("Received a fake Process\n");
                    //continue;
                }
            }
        }

        if (*shmId <= 0 && isProcessRunNow) //running process is finish
        {
            printf("FCFS : i will end process now\n");
            finishProcess(running);
            isProcessRunNow = 0;
        }
        // to do pick a new process

        switch (chosenPolicy)
        {
        case 1:
            if (firstfit())
            {
                isProcessRunNow = 1;
            }
            if (!priorityQueueIsEmpty(pq) && !isProcessRunNow)
            {
                printf("FCFS : i will pick process now\n");
                struct Process front = priorityQueuePop(pq);
                *shmId = front.remningTime + 1;
                running = front;
                if (firstFitAllocate(&running))
                {
                    isProcessRunNow = 1;
                    running.waitingTime = getClk() - running.arrivalTime;
                    fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                    running.pid = startProcess(running);
                }
            }

            break;
        case 2:
            if (nextfit())
                isProcessRunNow = 1;
            if (!priorityQueueIsEmpty(pq) && !isProcessRunNow)
            {
                printf("FCFS : i will pick process now\n");
                struct Process front = priorityQueuePop(pq);
                *shmId = front.remningTime + 1;
                running = front;
                if (nextfitallocatingmem(&running))
                {
                    isProcessRunNow = 1;
                    running.waitingTime = getClk() - running.arrivalTime;
                    fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                    running.pid = startProcess(running);
                }
            }
            break;
        default:
            break;
        }

        // to do : if it last process or the algorithm finish we will out from this loop done
        if (countProcess == numOfProcesses && priorityQueueIsEmpty(pq) && !isProcessRunNow)
            break;
    }
    printf("Schuder: hello i finished SJF...\n");
}

void SRTN()
{
    printf("Schuder: hello i started SRTN...\n");
    pq = (priorityQueue *)malloc(sizeof(priorityQueue));
    priorityQueueConstructor(pq);
    int time = -1;
    Process running;
    *shmId = -1;
    isProcessRunNow = 0;
    while (1)
    {

        processRecv.valid = 0; //clear prev recv mess
        processRecv.mtype = 1;
        if (countProcess < numOfProcesses)
        {

            printf("SRTN : i will receive now\n");
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
                    processRecv.remningTime = processRecv.runTime;
                    printf("%d %d %d %d RecvTime:%d\n", processRecv.id, processRecv.arrivalTime, processRecv.runTime, processRecv.priority, getClk());
                    priorityQueuePush(pq, processRecv, processRecv.runTime);
                    printPriorityQueue(pq);
                }
                else
                {
                    printf("Received a fake Process\n");
                }
            }
        }

        if (*shmId <= 0 && isProcessRunNow) //running process is finish
        {
            printf("SRTN : i will end process now");
            finishProcess(running);
            isProcessRunNow = 0;
        }

        switch (chosenPolicy)
        {
        case 1:
            if (firstfit())
            {
                isProcessRunNow = 1;
            }
            if (!isProcessRunNow && !priorityQueueIsEmpty(pq))
            {
                if (pq->head->data.remningTime < pq->head->data.runTime)
                {
                    printf("SRTN : i will continue process now\n");
                    struct Process front = priorityQueuePop(pq);
                    *shmId = front.remningTime + 1;
                    running = front;
                    isProcessRunNow = 1;
                    continueProcess(running);
                }
                else
                {
                    printf("SRTN : i will pick process now\n");
                    struct Process front = priorityQueuePop(pq);
                    *shmId = front.remningTime + 1;
                    running = front;
                    if (firstFitAllocate(&running))
                    {
                        isProcessRunNow = 1;
                        running.waitingTime = getClk() - running.arrivalTime;
                        fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                        running.pid = startProcess(running);
                    }
                }
            }
            break;
        case 2:
            if (nextfit())
                isProcessRunNow = 1;
            if (!isProcessRunNow && !priorityQueueIsEmpty(pq))
            {
                if (pq->head->data.remningTime < pq->head->data.runTime)
                {
                    printf("SRTN : i will continue process now\n");
                    struct Process front = priorityQueuePop(pq);
                    *shmId = front.remningTime + 1;
                    running = front;
                    isProcessRunNow = 1;
                    continueProcess(running);
                }
                else
                {
                    printf("SRTN : i will pick process now\n");
                    struct Process front = priorityQueuePop(pq);
                    *shmId = front.remningTime + 1;
                    running = front;
                    if (firstFitAllocate(&running))
                    {
                        isProcessRunNow = 1;
                        running.waitingTime = getClk() - running.arrivalTime;
                        fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                        running.pid = startProcess(running);
                    }
                }
            }
            break;
        default:
            break;
        }

        if (!priorityQueueIsEmpty(pq) && isProcessRunNow && *shmId > pq->head->data.remningTime) //no finish yet(stop it and push it pack to queue)
        {
            if (running.remningTime >= running.runTime)
            {
                running.remningTime = *shmId - 1;
            }
            else
                running.remningTime = *shmId;
            priorityQueuePush(pq, running, running.remningTime);
            stopProcess(running);
            isProcessRunNow = 0;
        }

        /* while (time == getClk())
            ;
        time = getClk();*/
        if (isProcessRunNow)
        {
            running.remningTime = *shmId;
        }

        // to do : if it last process or the algorithm finish we will out from this loop done
        if (countProcess == numOfProcesses && priorityQueueIsEmpty(pq) && !isProcessRunNow)
            break;
    }
    printf("Schuder: hello i finished SRTN...\n");
}

void HPF()
{
    printf("Schuder: hello i started HPF...\n");
    pq = (priorityQueue *)malloc(sizeof(priorityQueue));
    priorityQueueConstructor(pq);
    int time = -1;
    Process running;
    *shmId = -1;
    isProcessRunNow = 0;
    while (1)
    {

        processRecv.valid = 0; //clear prev recv mess
        processRecv.mtype = 1;
        if (countProcess < numOfProcesses)
        {

            printf("HPF : i will receive now\n");
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
                    processRecv.remningTime = processRecv.runTime;
                    printf("%d %d %d %d RecvTime:%d\n", processRecv.id, processRecv.arrivalTime, processRecv.runTime, processRecv.priority, getClk());
                    priorityQueuePush(pq, processRecv, processRecv.priority);
                    // printPriorityQueue(pq);
                }
                else
                {
                    printf("Received a fake Process\n");
                }
            }
        }

        if (*shmId <= 0 && isProcessRunNow) //running process is finish
        {
            printf("HPF : i will end process now");
            finishProcess(running);
            isProcessRunNow = 0;
        }
        if (!priorityQueueIsEmpty(pq) && isProcessRunNow && running.priority > pq->head->data.priority) //no finish yet(stop it and push it pack to queue)
        {
            if (running.remningTime >= running.runTime)
            {
                running.remningTime = *shmId - 1;
            }
            else
                running.remningTime = *shmId;
            priorityQueuePush(pq, running, running.priority);
            stopProcess(running);
            isProcessRunNow = 0;
        }
        switch (chosenPolicy)
        {
        case 1:
            if (firstfit())
            {
                isProcessRunNow = 1;
            }
            if (!isProcessRunNow && !priorityQueueIsEmpty(pq))
            {
                if (pq->head->data.remningTime < pq->head->data.runTime)
                {
                    printf("SRTN : i will continue process now\n");
                    struct Process front = priorityQueuePop(pq);
                    *shmId = front.remningTime + 1;
                    running = front;
                    isProcessRunNow = 1;
                    continueProcess(running);
                }
                else
                {
                    printf("SRTN : i will pick process now\n");
                    struct Process front = priorityQueuePop(pq);
                    *shmId = front.remningTime + 1;
                    running = front;
                    if (firstFitAllocate(&running))
                    {
                        isProcessRunNow = 1;
                        running.waitingTime = getClk() - running.arrivalTime;
                        fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                        running.pid = startProcess(running);
                    }
                }
            }
            break;
        case 2:
            if (nextfit())
                isProcessRunNow = 1;
            if (!isProcessRunNow && !priorityQueueIsEmpty(pq))
            {
                if (pq->head->data.remningTime < pq->head->data.runTime)
                {
                    printf("SRTN : i will continue process now\n");
                    struct Process front = priorityQueuePop(pq);
                    *shmId = front.remningTime + 1;
                    running = front;
                    isProcessRunNow = 1;
                    continueProcess(running);
                }
                else
                {
                    printf("SRTN : i will pick process now\n");
                    struct Process front = priorityQueuePop(pq);
                    *shmId = front.remningTime + 1;
                    running = front;
                    if (firstFitAllocate(&running))
                    {
                        isProcessRunNow = 1;
                        running.waitingTime = getClk() - running.arrivalTime;
                        fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), running.memSize, running.id, running.startIndex, running.endIndex);
                        running.pid = startProcess(running);
                    }
                }
            }
            break;
        default:
            break;
        }
        /* if (!isProcessRunNow && !priorityQueueIsEmpty(pq))
        {
            if (pq->head->data.remningTime < pq->head->data.runTime)
            {
                printf("HPF : i will continue process now\n");
                struct Process front = priorityQueuePop(pq);
                *shmId = front.remningTime + 1;
                running = front;
                isProcessRunNow = 1;
                continueProcess(running);
            }
            else
            {
                printf("HPF : i will pick process now\n");
                struct Process front = priorityQueuePop(pq);
                *shmId = front.remningTime + 1;
                running = front;
                isProcessRunNow = 1;
                running.pid = startProcess(running);
            }
        }*/

        /* while (time == getClk())
            ;
        time = getClk();*/
        if (isProcessRunNow)
        {
            running.remningTime = *shmId;
        }

        // to do : if it last process or the algorithm finish we will out from this loop done
        if (countProcess == numOfProcesses && priorityQueueIsEmpty(pq) && !isProcessRunNow)
            break;
    }
    printf("Schuder: hello i finished HPF...\n");
}

void cleanup(int signum)
{
    printf("Terminate shared memory from scheduler\n");
    shmctl(shmid, IPC_RMID, NULL);
    //shmctl(msgQ, IPC_RMID, NULL);
    msgctl(msgQ, IPC_RMID, (struct msqid_ds *)0);
    kill(getpid(), SIGKILL);
}

bool nextfitallocatingmem(Process *p)
{
    int startpos = 0;
    int counter = 0;

    allocate = 0;
    nomem = 0;
    if ((pointertomem + p->memSize) < 1024)
    {
        startpos = pointertomem;
    }
    else
    {
        startpos = 0;
    }

    for (int i = startpos; i < 1024; i++)
    {
        if (memory[i] == 0)
        {
            allocate = 1;
            nomem = 0;
            counter++;
            if (counter == p->memSize - 1)
            {
                break;
            }
        }
        else
        {
            allocate = 0;
            nomem = 1;
            counter = 0;
        }
    }

    if (allocate)
    {
        p->startIndex = pointertomem;
        p->endIndex = p->memSize + pointertomem;
        linkedlistdelete(waitList, p->id);
        for (int i = pointertomem; i < p->memSize; i++)
        {
            memory[i] = 1;
            if (i < 1023)
            {
                pointertomem = i + 1;
            }
            else
            {
                pointertomem = 0;
            }
        }
        return true;
    }
    else if (nomem)
    {
        if (!inList(waitList, *p))
        {
            linkedlistPush(waitList, *p);
        }
        return false;
    }
}

void nextfitdeallocatingmem(Process *p)
{
    for (int i = p->startIndex; i < p->endIndex; i++)
    {
        memory[i] = 0;
    }
    if (p->startIndex + p->memSize == pointertomem)
    {
        pointertomem = p->startIndex;
    }
}

bool nextfit()
{
    flag = 0;
    node *temp = waitList->head;
    while (temp != NULL && !isProcessRunNow)
    {
        if (nextfitallocatingmem(&temp->data))
        {
            printf("FCFS : i will pick process now\n");
            *shmId = temp->data.remningTime + 1;
            flag = 1;
            temp->data.waitingTime = getClk() - temp->data.arrivalTime;
            fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), temp->data.memSize, temp->data.id, temp->data.startIndex, temp->data.endIndex);
            temp->data.pid = startProcess(temp->data);
        }
    }

    return flag;
}

bool firstFitAllocate(Process *p)
{
    int freeMemory = 0, startIndex = -1;
    for (int i = 0; i < 1024; ++i)
    {
        if (freeMemory == 0 && memoryFirstFit[i] == 0)
            startIndex = i;
        if (memoryFirstFit[i] == 0)
            freeMemory++;
        else
        {
            freeMemory = 0;
            startIndex = -1;
        }
        if (freeMemory == p->memSize)
            break;
    }
    if (startIndex != -1 && freeMemory >= p->memSize)
    {
        linkedlistdelete(waitingListFirstFit, p->id);
        p->startIndex = startIndex;
        p->endIndex = p->startIndex + p->memSize;
        p->status = Ready;
        for (int i = startIndex; i < freeMemory; ++i)
            memoryFirstFit[i] = 1;
        return true;
    }
    else
    {
        if (!inList(waitingListFirstFit, *p))
        {
            linkedlistPush(waitingListFirstFit, *p); // could be wrong
        }
        return false;
    }
}
bool firstFitDeallocate(Process *p)
{
    for (int i = p->startIndex; i < p->endIndex; ++i)
    {
        memoryFirstFit[i] = 0;
    }
    return true;
}

bool firstfit()
{
    flag = 0;
    node *temp = waitingListFirstFit->head;
    while (temp != NULL && !isProcessRunNow)
    {
        if (firstFitAllocate(&temp->data))
        {
            printf("FCFS : i will pick process now\n");
            *shmId = temp->data.remningTime + 1;
            flag = 1;
            temp->data.waitingTime = getClk() - temp->data.arrivalTime;
            fprintf(MemoryLog, "at time %d allocated %d bytes for process %d from %d to %d\n", getClk(), temp->data.memSize, temp->data.id, temp->data.startIndex, temp->data.endIndex);
            temp->data.pid = startProcess(temp->data);
        }
    }
    return flag;
}

/* 
// should we increase the waiting time for the waitinglist processes?
// allocating
    node* temp = waitingList->head;
    while(temp != NULL) {
        FFAllocate(temp->data);
    }
    FFAllocate(thisProc);
*/

/*  
// deallocating
    FFDeallocate(thisProc);
    node* temp = waitingList->head;
    while(temp != NULL) {
        FFAllocate(temp->data);
    }


*/
