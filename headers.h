#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300

enum processStatus
{
    Running,
    Ready,
    Blocked
};

struct processHeaders
{
    long algorithm;
    long numOfProcesses;
    long processParameter;
    long mtype;
} procHeaders;

struct Process
{
    int id;
    int arrivalTime;
    int runTime;
    int priority;
    int pid;
    enum processStatus status;
};

struct node
{
    Process data;
    node *next;
};
void nodeConstructor(node *n, Process p)
{
    n = malloc(sizeof node);
    n->data = p;
    n->next = NULL;
}
///==============================
//don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All processes call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    printf("Clock Starting...\n");
    int shmid = shmget(SHKEY, 4, 0444);
    while ((long)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All processes call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

struct Queue
{
    // Initialize front and rear
    node *head, *tail;
    int size;                     // size of the queue
} void queueConstructor(Queue *q) // consturctor of the queue
{
    head = tail = NULL;
    size = 0;
}
bool queueIsEmpty(Queue *q) // check if is empty
{
    return !(q->size);
}
void queuePush(Queue *q, Process p) // push new process
{
    q->size++;
    node *temp;
    nodeConstructor(temp, p);
    if (queueIsEmpty(q))
    {
        q->head = q->tail = temp;
    }
    else
    {
        q->head->next = temp;
        q->head = temp;
    }
}
Process queuePop(Queue *q) // pop the process in the top
{
    Process p;
    if (queueIsEmpty(q))
        return NULL;
    node *temp = q->tail;
    q->tail = q->tail->next;
    p = temp->data;
    free(temp);
    return p;
}
Process queueTop(Queue *q) // get the process in the top
{
    Process p;
    if (queueIsEmpty(q))
        return NULL;
    node *temp = q->tail;
    p = temp->data;
    return p;
}