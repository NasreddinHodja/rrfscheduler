#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_Q 1000
#define QUANTUM 1

enum PRIORITY {low_priority, normal_priority, high_priority};
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

int new_pid() {
    int pid = PROC_COUNT++;
    PROC_COUNT = PROC_COUNT % MAX_Q;
    return pid;
}

Process* p_create(int pid, int ppid, int status, int priority) {
  Process* p = (Process*) malloc(sizeof(Process));
  if(pid == -1) pid = new_pid();
  if(ppid == -1) ppid = 0;
  p->pid = pid;
  p->ppid = 0;
  p->status = status;
  p->priority = priority;
  return p;
}

Process* p_fork(Process* p, int status, int priority) {
  Process* child = (Process*) malloc(sizeof(Process));
  child->pid = new_pid();
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

Queue* q_create(Process** a, int size) {
  Queue* q = (Queue*) malloc(sizeof(Queue));
  q->front = 0;
  q->back = size;
  q->size = size;
  if(a == NULL) return q;

  for(int i = 0; i < size; i++)
    q->queue[i] = a[i];
  return q;
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

void init() {
  PROC_COUNT = 0;
}

int main() {
  init();

  Process* p = p_create(-1, -1, running, low_priority);
  printf("%s\n", p_to_string(p_fork(p, waiting, low_priority)));

  return 0;
}
