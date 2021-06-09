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
    int remningTime;
    int priority;
    int pid;
    int valid; //use it for send/recieve
    int mtype; //use it for send/recieve
    int sendTime;
    int waitingTime;
    int stopTime;
    int memSize;
<<<<<<< HEAD
    int startIndex;   //start of process in memory
    int endIndex;
=======
>>>>>>> 2743ee2a129d2ef7e09dda54b9d22f4b71bbaf17
    enum processStatus status;
};
typedef struct Process Process;
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
    int shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
    while ((long)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
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

struct node
{
    struct Process data;
    struct node *next;
};
typedef struct node node;
void nodeConstructor(node *n, Process p)
{
    n->data.id = p.id;
    n->data.arrivalTime = p.arrivalTime;
    n->data.runTime = p.runTime;
    p.remningTime = p.remningTime;
    n->data.remningTime = p.remningTime;
    n->data.valid = p.valid;
    n->data.status = p.status;
    n->data.priority = p.priority;
    n->data.pid = p.pid;
    n->data.memSize=p.memSize;
    n->data.startIndex=p.startIndex;
    n->data.endIndex=p.endIndex;
    
    n->next = NULL;
}
struct Queue
{
    //Initialize front and rear
    node *head, *tail;
    int size; // size of the queue
};
typedef struct Queue Queue;
void queueConstructor(Queue *q) // consturctor of the queue
{
    q->head = q->tail = NULL;
    q->size = 0;
}
bool queueIsEmpty(Queue *q) // check if is empty
{
    if (q == NULL || q->head == NULL)
        return true;
    return false;
    //return !(q->size);
    //printf("queue constrctor end empty\n");
}
void queuePush(Queue *q, Process p) // push new process
{
    node *temp = (node *)malloc(sizeof(node));
    q->size = q->size + 1;
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
    Process p = {.id = -1};
    if (queueIsEmpty(q))
    {
        return p;
    }
    node *temp = q->tail;
    q->tail = q->tail->next;
    p = temp->data;
    free(temp);
    q->size = q->size - 1;
    if (q->tail == NULL)
        q->head = NULL;
    return p;
}
Process queueTop(Queue *q) // get the process in the top
{
    Process p;
    if (queueIsEmpty(q))
        return p;
    node *temp = q->tail;
    p = temp->data;
    return p;
}
void printQueue(Queue *q)
{
    node *temp = q->tail;
    while (temp != NULL)
    {
        printf("id %d ,", temp->data.id);
        temp = temp->next;
    }
    printf("\n");
}
//2-Priority Queue
/*
priorityQueue * q;
priorityQueueConstructor(q);
Process p;
priorityQueuePush(q,p,pr);
*/
struct nodeWithPriority
{
    Process data;
    struct nodeWithPriority *next;
    int priority; //lower value -> high priority
};
typedef struct nodeWithPriority nodeWithPriority;
void newNodeWithPriority(nodeWithPriority *n, Process p, int pr)
{
    n->data.id = p.id;
    n->data.arrivalTime = p.arrivalTime;
    n->data.runTime = p.runTime;
    p.remningTime = p.remningTime;
    n->data.remningTime = p.remningTime;
    n->data.valid = p.valid;
    n->data.status = p.status;
    n->data.priority = p.priority;
    n->data.pid = p.pid;
    n->next = NULL;
    n->priority = pr;
}
struct priorityQueue
{
    nodeWithPriority *head;
    int size;
};
typedef struct priorityQueue priorityQueue;
void priorityQueueConstructor(priorityQueue *q) // consturctor of the queue
{
    q->head = NULL;
    q->size = 0;
}
bool priorityQueueIsEmpty(priorityQueue *q)
{
    if (q == NULL || q->head == NULL)
        return true;
    return false;
}
void priorityQueuePush(priorityQueue *q, Process p, int pr)
{

    nodeWithPriority *tmp = (nodeWithPriority *)malloc(sizeof(nodeWithPriority));
    newNodeWithPriority(tmp, p, pr);

    if (priorityQueueIsEmpty(q))
        q->head = tmp;
    else if (q->head->priority > pr) //the insterd process is best one
    {
        tmp->next = q->head;
        q->head = tmp;
    }
    else
    {
        nodeWithPriority *s = q->head;
        while (s->next != NULL && s->next->priority < pr)
            s = s->next;
        tmp->next = s->next;
        s->next = tmp;
    }

    q->size++;
}

Process priorityQueuePop(priorityQueue *q)
{
    Process p;
    if (priorityQueueIsEmpty(q))
        return p;
    q->size--;
    nodeWithPriority *tmp = q->head;
    q->head = q->head->next;
    p = tmp->data;
    free(tmp);
    return p;
}
Process priorityQueueTop(priorityQueue *q)
{
    return q->head->data;
}
void printPriorityQueue(priorityQueue *q)
{
    nodeWithPriority *temp = q->head;
    while (temp != NULL)
    {
        printf("id %d ,", temp->data.id);
        temp = temp->next;
    }
    printf("\n");
}

int min(int a, int b)
{
    if (a < b)
        return a;
    return b;
}

/*3-linked list
	linkedlist * ll;
	linkedlistconstructor(q);
	Process p;
	linkedlistPush(q,p,pr);
	linkedlistdelete(p);
*/

struct linkedlist
{
    //Initialize front and rear
    node *head, *tail;
    int size; // size of the queue
};
typedef struct linkedlist linkedlist;
void linkedlistConstructor(linkedlist *ll) // consturctor of the queue
{
    ll->head = ll->tail = NULL;
    ll->size = 0;
}
bool linkedlistIsEmpty(linkedlist *ll) // check if is empty
{
    if (ll == NULL || ll->head == NULL)
        return true;
    return false;

}
void linkedlistPush(linkedlist *ll, Process p) // push new process
{
    node *temp = (node *)malloc(sizeof(node));
    ll->size = ll->size + 1;
    nodeConstructor(temp, p);
    if (linkedlistIsEmpty(ll))
    {
        ll->head = ll->tail = temp;
    }
    else
    {
        ll->tail->next = temp;
        ll->tail = temp;
    }
}
Process linkedlistdelete(linkedlist *ll,int pid) // pop the process in the top
{
    Process p = {.id = -1};
    if (linkedlistIsEmpty(ll))
    {
        return p;
    }
    
    node *temp = ll->head;
    node *prev= NULL;
    while(temp!=NULL)
    {
    	prev=temp;
    	if(temp->data.pid==p.pid)
    		break;
    	
    	temp=temp->next;
    }
    if(temp==NULL)
    	return p;
    	
    p = temp->data;
    prev->next=temp->next;
    temp->next=NULL;
    free(temp);
    ll->size = ll->size - 1;
    if (ll->head == NULL)
        ll->tail = NULL;
    return p;
}
bool inList(linkedlist *ll,Process p)
{
	if (linkedlistIsEmpty(ll))
    {
        return false;
    }
    node *temp = ll->head;
    while(temp!=NULL)
    {
    	if(temp->data.id==p.id)
    	{
    		return true;
    	}
    	temp=temp->next;
    }
    return false;
}

void printlinkedlist(linkedlist *ll)
{
    node *temp = ll->head;
    while (temp != NULL)
    {
        printf("id %d ,", temp->data.id);
        temp = temp->next;
    }
    printf("\n");
}
