#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_Q 1000
#define MAX_E 1000
#define MAX_IO_T 7
#define QUANTUM 1
#define MAX_SERVICE_T 17

enum PRIORITY {high_priority, normal_priority, low_priority};
enum IO {disk, mag_tape, printer};
enum PROC_STATUS {waiting, ready, running};

int PROC_COUNT;

typedef struct Process {
  int pid;
  int ppid;
  int status;
  int priority;
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
  int service_time;
  int* io;
} SchedulerEntry;

typedef struct Scheduler {
  Queue* queues[3];
  SchedulerEntry entries[MAX_Q];
} Scheduler;

SchedulerEntry* se_create(Process* p, int begin, int service_time, int* io) {
  SchedulerEntry* se = (SchedulerEntry*) malloc(sizeof(SchedulerEntry));
  se->p = p;
  se->begin = begin;
  se->service_time = service_time;
  for(int i = 0; i < service_time; i++)
    se->io[i] = io[i];
  return se;
}

int gen_pid() {
  int pid = PROC_COUNT++;
  PROC_COUNT = PROC_COUNT % MAX_Q;
  return pid;
}

int rand_duration(bool service_time) {
  if(service_time) return random() % MAX_SERVICE_T;
  return random() % MAX_IO_T;
}

Process* p_create(int pid, int ppid, int status, int priority) {
  Process* p = (Process*) malloc(sizeof(Process));
  if(pid == -1) pid = gen_pid();
  if(ppid == -1) ppid = 0;
  p->pid = pid;
  p->ppid = 0;
  p->status = status;
  p->priority = priority;
  return p;
}

Process* p_fork(Process* p, int status, int priority) {
  Process* child = (Process*) malloc(sizeof(Process));
  child->pid = gen_pid();
  child->ppid = p->pid;
  child->status = status;
  child->priority = priority;
  return child;
}

char* p_to_string(Process* p) {
  char* a = (char*) malloc(100 * sizeof(char*));
  sprintf(a, "{ PID: %d, PPID: %d, status: %d, priority: %d }",
          p->pid, p->ppid, p->status, p->priority);
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
  s->queues = queues;
  s->entries = entries;
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
                        q->queue[q->front]->status, q->queue[q->front]->priority);
  free(q->queue[q->front]);
  q->queue[q->front++] = NULL;
  return p;
}

void schedule() {
  while(true) {

  }
}

void init() {
  PROC_COUNT = 0;
}

int main() {
  init();

  SchedulerEntry entries[10] = {se_entry()};

  Scheduler* scheduler = s_create(NULL, entries)

  return 0;
}
