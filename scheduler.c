#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_Q 100
#define MAX_P 100
#define QUANTUM 2
#define MAX_IO_T 5
#define MAX_SERVICE_T 10

enum PRIORITY {high_priority, normal_priority, low_priority};
enum IO {disk, mag_tape, printer};
enum PROC_STATUS {waiting, ready, running};

int PROC_COUNT;

typedef struct Process {
  int pid;
  int ppid;
  int status;
} Process;

typedef struct Queue {
  Process* queue[MAX_Q];
  int front;
  int back;
  int size;
} Queue;

typedef struct SchedulerEntry {
  Process* p;
  int begin;
  int priority;
  int service_time;
  int* io;
} SchedulerEntry;

typedef struct Scheduler {
  Queue* queues[3];
  SchedulerEntry entries[MAX_P];
} Scheduler;

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

int gen_pid() {
  int pid = PROC_COUNT++;
  PROC_COUNT = PROC_COUNT % MAX_P;
  return pid;
}

int rand_duration(bool service_time) {
  if(service_time) return random() % MAX_SERVICE_T;
  return random() % MAX_IO_T;
}

Process* p_create(int pid, int ppid, int status) {
  Process* p = (Process*) malloc(sizeof(Process));
  if(pid == -1) pid = gen_pid();
  if(ppid == -1) ppid = 0;
  p->pid = pid;
  p->ppid = 0;
  return p;
}

// TODO make SchedulerEntry instead of Process
/* Process* p_fork(Process* p, int status) { */
/*   p->status = status; */
/*   Process* child = (Process*) malloc(sizeof(Process)); */
/*   child->pid = gen_pid(); */
/*   child->ppid = p->pid; */
/*   child->status = status; */
/*   return child; */
/* } */

char* p_to_string(Process* p) {
  char* a = (char*) malloc(100 * sizeof(char*));
  sprintf(a, "{ PID: %d, PPID: %d, status: %d }",
          p->pid, p->ppid, p->status);
  return a;
}

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

Scheduler* s_create(Queue* queues[3], SchedulerEntry* entries) {
  Scheduler* s = (Scheduler*) malloc(sizeof(Scheduler));
  if(queues == NULL){
    for(int i = 0; i < 3; i++)
      queues[i] = q_create(NULL, 0);
  }
  for(int i = 0; i < 3; i++)
    s->queues[i] = queues[i];
  for(int i = 0; i < MAX_P; i++)
    s->entries[i] = entries[i];
}

int q_next_idx(Queue* q, int idx) {
  int i = idx;
  do {
    i = (i + 1) % (q->back);
  } while(q->queue[i] == NULL && i != q->front);
  return i;
}

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

bool q_push(Queue* q, Process* p) {
  if(q->back == MAX_Q || p == NULL) return false;
  q->size++;
  q->back++;
  q->queue[q->back-1] = p;
  return true;
}

Process* q_pop(Queue* q) {
  if(q->size == 0) return NULL;
  q->size--;
  Process* p = p_create(q->queue[q->front]->pid, q->queue[q->front]->pid,
                        q->queue[q->front]->status);
  free(q->queue[q->front]);
  q->queue[q->front++] = NULL;
  return p;
}

/* void schedule() { */
/*   while(true) { */

/*   } */
/* } */

void init() {
  PROC_COUNT = 0;
}

int main() {
  init();

  // TODO make a test case
  int ios_p1[5] = {0, 0, 0, 0, 0};
  SchedulerEntry* p1_entry = se_create(p_create(-1, -1, ready), 0, low_priority, 5, ios_p1);

  /* Scheduler* scheduler = s_create(NULL, entries) */

  return 0;
}
