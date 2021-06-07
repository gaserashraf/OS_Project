#include "headers.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
void clearResources(int);

int msgQ;
struct Process *processes;
#define n 2

int *shmIdTerm, shmidterm; //for the running process
int main(int argc, char *argv[])
{

    signal(SIGINT, clearResources);
    char *fileName;
    int chosenAlgorithm = 5;
    int processParam = 1;
    // 1. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    /* if (argc < 3)
    {
        perror("Invlaid number of arguments");
        exit(-1);
    }
    else
    {
        strcpy(fileName, argv[1]);
        chosenAlgorithm = atoi(argv[3]);
        if (argc > 3)
            processParam = atoi(argv[5]);
    }*/

    /* key_t sharedMemKey = ftok("Makefile", 65);
    shmidterm = shmget(sharedMemKey, 4000, 0666 | IPC_CREAT); // crete shared
    if (shmid == -1)
    {
        perror("Error in creating message queue");
        return -1;
    }
    shmIdTerm = (int *)shmat(shmid, (void *)0, 0);
    *shmIdTerm = false;*/

    // 5. Create a data structure for processes and provide it with its parameters.
    processes = (struct Process *)malloc(100 * sizeof(struct Process));
    // 2. Read the input files.
    FILE *processesInput;
    processesInput = fopen("processes.txt", "r");
    if (processesInput == NULL)
    {
        perror("Error While reading processes.txt file\n");
        exit(-1);
    }
    int a, b, c, d;
    //TODO: Change this method to skip first line
    char q[10], w[10], e[10], r[10];
    fscanf(processesInput, "%s %s %s %s", q, w, e, r);
    int numOfProcesses = 0;
    while (fscanf(processesInput, "%d %d %d %d", &a, &b, &c, &d) != -1)
    {
        processes[numOfProcesses].id = a;
        processes[numOfProcesses].arrivalTime = b;
        processes[numOfProcesses].runTime = c;
        processes[numOfProcesses].remningTime = c;
        processes[numOfProcesses].priority = d;
        processes[numOfProcesses].waitingTime = -1;
        numOfProcesses++;
    }
    char numProcesses[500];
    sprintf(numProcesses, "%d", numOfProcesses);

    // 3. Initiate and create the scheduler and clock processes.
    int clkProcess = fork();
    if (clkProcess == -1)
    {
        perror("Error in initializing Clock Child\n");
    }
    else if (clkProcess == 0)
    {
        execl("clk.out", "clk", NULL);
        //exit(0);
    }

    int schedulerProcess = fork();
    if (schedulerProcess == -1)
    {
        perror("Error in initializing schedular Child\n");
    }
    //TODO Send procHeaders while executing
    else if (schedulerProcess == 0)
    {
        printf("hello i will fork scheduler...\n");
        system("gcc scheduler.c -o scheduler.out");
        /* if (argc > 3)
            execl("scheduler.out", "scheduler", argv[3], numProcesses, argv[5], NULL);
        else
            execl("scheduler.out", "scheduler", argv[3], numProcesses, NULL);*/
        execl("scheduler.out", "scheduler", NULL);
    }
    FILE *f = fopen("key", "r");
    key_t key_id = ftok("key", 'a');
    msgQ = msgget(key_id, 0666 | IPC_CREAT);
    if (msgQ == -1)
    {
        perror("Error in creating message queue");
        return -1;
    }
    else
    {
        printf("Process Generator:msgQ created Successfully with id = %d\n", msgQ);
    }

    /* procHeaders.mtype = 1;
    procHeaders.algorithm = chosenAlgorithm;
    procHeaders.numOfProcesses = numOfProcesses;
    procHeaders.processParameter = processParam;
    int val = msgsnd(msgQ, &procHeaders, sizeof(procHeaders.algorithm) + sizeof(procHeaders.numOfProcesses) + sizeof(procHeaders.processParameter), !IPC_NOWAIT);
    if (val == -1)
        perror("Errror in send\n");*/

    // 4. Use this function after creating the clock Process to initialize clock.
    initClk();
    int i = 0;
    while (i < numOfProcesses)
    {
        int temp = processes[i].arrivalTime;
        processes[i].valid = false;
        if (temp > getClk())
        {
            sleep(1);
            printf("%d\n", getClk());
            int val = msgsnd(msgQ, &processes[i], sizeof(processes[i]), !IPC_NOWAIT);
            if (val == -1)
                printf("Errror in send Process#%d\n", i);
        }
        else
        {
            // 6. Send the information to the scheduler at the appropriate time.
            processes[i].valid = true;
            processes[i].sendTime = temp;
            printf("Sending Process#%d\n", i);
            int val = msgsnd(msgQ, &processes[i], sizeof(processes[i]), !IPC_NOWAIT);
            if (val == -1)
                printf("Errror in send Process#%d\n", i);
            i++;
        }
    }

    // TODO Generation Main Loop
    // 7. Clear clock resources
    sleep(10000);
    destroyClk(false);
}

void clearResources(int signum)
{
    //printf("Terminate msgQ from generator\n");
    //msgctl(msgQ, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
    kill(getpid(), SIGKILL);
    //signal(SIGINT, clearResources);
}
