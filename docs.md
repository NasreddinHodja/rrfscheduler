
# Table of Contents

1.  [Includes](#orgd3b0a31)
2.  [Defines, enums and global variables](#org0fb4fe1)
3.  [Structures and associated functions](#org0ac4dc2)
    1.  [`Process`](#org3cfc6b4)
        1.  [`p_create`](#org6479c35)
        2.  [`p_to_string`](#org81a6986)
    2.  [`Queue`](#org343d66d)
        1.  [`q_create`](#orgc7c401b)
        2.  [`q_next_idx`](#orgd12dea4)
        3.  [`q_print`](#org7b5f1b7)
        4.  [`q_push`](#orgdab4340)
        5.  [`q_pop`](#org4bcd03e)
    3.  [`SchedulerEntry`](#org500c137)
        1.  [`se_create`](#org34ff230)
    4.  [`Scheduler`](#org0c778cf)
4.  [Utils](#org102f090)
    1.  [`gen_pid`](#org3092942)
    2.  [`rand_duration`](#org505f42f)



<a id="orgd3b0a31"></a>

# Includes

-   `stdio`: for `I/O`
-   `stdlib`: for memory manipulation (`malloc`, `free`)
-   `stdbool`: for the usage of `true` e `false`
-   `unistd`: randomization

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <unistd.h>


<a id="org0fb4fe1"></a>

# Defines, enums and global variables

-   `MAX_Q`: maximum size for a `Queue`
-   `MAX_P`: maximum quantity of simulated processes
-   `QUANTUM`: duration of a quantum (seconds)
-   `MAX_IO_T`: maximum duration of an `I/O` operation (quantums)
-   `MAX_SERVICE_T`: maximum service time of a process (quantums)
-   `PRIORITY`: possible process priorities
-   `IO`: types of `I/O`
-   `PROC_STATUS`: possible process states
-   `PROC_COUND`: process counter

    #define MAX_Q 1000
    #define MAX_P 1000
    #define QUANTUM 1
    #define MAX_IO_T 7
    #define MAX_SERVICE_T 17
    
    enum PRIORITY {high_priority, normal_priority, low_priority};
    enum IO {disk, mag_tape, printer};
    enum PROC_STATUS {waiting, ready, running};
    
    int PROC_COUNT;


<a id="org0ac4dc2"></a>

# Structures and associated functions


<a id="org3cfc6b4"></a>

## `Process`

`struct` that simulates a process.

-   `pid` (`int`): unique id of process
-   `ppid` (`int`): unique id of parent process
-   `status` (`PROC_STATUS`): process status

    typedef struct Process {
      int pid;
      int ppid;
      int status;
    } Process;


<a id="org6479c35"></a>

### `p_create`

Creates a new `Process` with given arguments as fields.

-   **in**
    -   `pid` (`int`): unique id of process
    -   `ppid` (`int`): unique id of parent process
    -   `status` (`PROC_STATUS`): process status
-   **out** (`Process*`): pointer to the `Process` created

    Process* p_create(int pid, int ppid, int status) {
      Process* p = (Process*) malloc(sizeof(Process));
      if(pid == -1) pid = gen_pid();
      if(ppid == -1) ppid = 0;
      p->pid = pid;
      p->ppid = 0;
      return p;
    }


<a id="org81a6986"></a>

### `p_to_string`

Generates a string representation of the given `Process`.

-   **in**
    -   `p` (`Process*`):
    -   `ppid` (`int`): id do parente do processo
    -   `status` (`PROC_STATUS`): status do processo
-   **out** (`Process*`): `Process` pointer created

    char* p_to_string(Process* p) {
      char* a = (char*) malloc(100 * sizeof(char*));
      sprintf(a, "{ PID: %d, PPID: %d, status: %d }",
              p->pid, p->ppid, p->status);
      return a;
    }


<a id="org343d66d"></a>

## `Queue`

FIFO of `Process` pointers.

-   `queue` (`Process*`): array of `Process` pointers, of size `MAX_Q`
-   `front` (`int`): index for the first element of the queue
-   `back` (`int`): index after the last element of the queue
-   `size` (`int`): number of elements in the queue

    typedef struct Queue {
      Process* queue[MAX_Q];
      int front;
      int back;
      int size;
    } Queue;


<a id="orgc7c401b"></a>

### `q_create`

Creates a new `Queue` with given arguments as fields.

-   **in**
    -   `procs` (`Process**`): array of `Process` pointers
    -   `size` (`int`): size of procs
-   **out** (`Queue*`): `Queue` pointer created

    Queue* q_create(Process** procs, int size) {
      Queue* q = (Queue*) malloc(sizeof(Queue));
      q->front = 0;
      q->back = size;
      q->size = size;
      if(procs == NULL) return q;
      for(int i = 0; i < size; i++)
        q->queue[i] = procs[i];
      return q;
    }


<a id="orgd12dea4"></a>

### `q_next_idx`

Given a `Queue*` `q` and an index `idx`, returns the index of next item in the queue.

-   **in**
    -   `q` (`Queue*`)
    -   `idx` (`int`)
-   **out** (`int`): index of next item

    int q_next_idx(Queue* q, int idx) {
      int i = idx;
      do {
        i = (i + 1) % (q->back);
      } while(q->queue[i] == NULL && i != q->front);
      return i;
    }


<a id="org7b5f1b7"></a>

### `q_print`

Print the `Queue` pointed by `q.`

-   **in**
    -   `q` (`Queue*`)
    -   `idx` (`int`)

    void q_print(Queue* q) {
      int i = q->front;
      printf("[ ");
      bool st = true;
      if(q->size != 0) {
        do {
          if(!st) printf(" ");
          printf("%d", q->queue[i]->pid);
          i = q_next_idx(q, i);
          st = false;
        } while(i != q->front);
      }
      printf(" ]\n");
    }


<a id="orgdab4340"></a>

### `q_push`

Pushes `Process` pointed by `p` to `Queue` pointed by `q.`

-   **in**
    -   `q` (`Queue*`)
    -   `p` (`Process*`)
-   **out** (`bool`): `true` if successful, `false` if else

    bool q_push(Queue* q, Process* p) {
      if(q->back == MAX_Q || p == NULL) return false;
      q->size++;
      q->back++;
      q->queue[q->back-1] = p;
      return true;
    }


<a id="org4bcd03e"></a>

### `q_pop`

Pops from `Queue` pointed by `q`.

-   **in**
    -   `q` (`Queue*`)
-   **out** (`Process*`): element popped from `q`, `NULL` if unsucsessful

    Process* q_pop(Queue* q) {
      if(q->size == 0) return NULL;
      q->size--;
      Process* p = p_create(q->queue[q->front]->pid, q->queue[q->front]->pid,
                            q->queue[q->front]->status);
      free(q->queue[q->front]);
      q->queue[q->front++] = NULL;
      return p;
    }


<a id="org500c137"></a>

## `SchedulerEntry`

Contains a pointer to a `Process`, along with information relevant to the scheduler.

-   `p` (`Process*`)
-   `begin` (`int`): moment in time where the process arrives
-   `priority` (`int`): process priority (`PRIORITY`)
-   `service_time` (`int`): process service<sub>time</sub> (quantums)
-   `io` (`int*`): array of size `service_time` containing types of `I/O` operations at the index corresponding to what moment in the service time where they ocur
    -   ex.: `{0, 1, 0, 2}` indicates that the process realizes a type `1` `I/O` operation after 1 quantum of execution, and a type `2` `I/O` operation after 3 quantums of execution

    typedef struct SchedulerEntry {
      Process* p;
      int begin;
      int priority;
      int service_time;
      int* io;
    } SchedulerEntry;


<a id="org34ff230"></a>

### `se_create`

Generates a `SchedulerEntry` for given process.

-   **in**
    -   `p` (`Process*`)
    -   `begin` (`int`)
    -   `priority` (`int`)
    -   `service_time` (`int`)
    -   `io` (`int*`)
-   **out** (`SchedulerEntry*`)

    SchedulerEntry* se_create(Process* p, int begin, int priority,
                              int service_time, int* io) {
      SchedulerEntry* se = (SchedulerEntry*) malloc(sizeof(SchedulerEntry));
      se->p = p;
      se->begin = begin;
      se->priority = priority;
      se->service_time = service_time;
      se->io = (int*) malloc(sizeof(int) * service_time);
      for(int i = 0; i < service_time; i++)
        se->io[i] = io[i];
      return se;
    }


<a id="org0c778cf"></a>

## `Scheduler`

Emulates a scheduler with 3 priority queues.

-   `queues` (`Queue**`): 3 priority queues
-   `entries` (`SchedulerEntry**`): array of size `MAX_P` containg the entries

    typedef struct Scheduler {
      Queue* queues[3];
      SchedulerEntry entries[MAX_P];
    } Scheduler;


<a id="org102f090"></a>

# Utils


<a id="org3092942"></a>

## `gen_pid`

Generates unique `PID`.

-   **out** (`int`): new unique `PID`

    int gen_pid() {
      int pid = PROC_COUNT++;
      PROC_COUNT = PROC_COUNT % MAX_P;
      return pid;
    }


<a id="org505f42f"></a>

## `rand_duration`

Generates random duration for service time or `I/O` operation time.

-   **in**
    -   `service_time` (`bool`): if `true`, indicates that a service time is needed
-   **out** (`int`): random service time, if `service_time` is `true`, or else random durantion of `I/O` operation

    int rand_duration(bool service_time) {
      if(service_time) return random() % MAX_SERVICE_T;
      return random() % MAX_IO_T;
    }

