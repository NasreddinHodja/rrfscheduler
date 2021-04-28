#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_Q 1000
#define QUANTUM 1

enum PRIORITY {low_priority, normal_priority, high_priority};
enum IO {disk, mag_tape, printer};

typedef struct Process {
  int id;
  int priority;
} Process;

typedef struct Queue {
  Process* queue[MAX_Q];
  int front;
  int back;
  int size;
} Queue;

Process* p_create(int id, int priority) {
  Process* p = (Process*) malloc(sizeof(Process));
  p->id = id;
  p->priority = priority;
  return p;
}

char* p_to_string(Process* p) {
  char* a;
  sprintf(a, "{ PID: %d, priority: %d }", p->id, p->priority);
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
      printf("%d", q->queue[i]->id);
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
  Process* p = p_create(q->queue[q->front]->id, q->queue[q->front]->priority);
  free(q->queue[q->front]);
  q->queue[q->front++] = NULL;
  return p;
}

int main() {
  /* Process* processes[4] = {p_create(1), p_create(2), p_create(3)}; */
  /* Queue* q = q_create(processes, sizeof(processes) / sizeof(Process*)); */
  /* Queue* q = q_create(NULL, 0); */
  /* q_push(q, p_create(1, low_priority)); */
  /* q_push(q, p_create(2, normal_priority)); */
  /* q_push(q, p_create(3, high_priority)); */

  /* q_print(q); */

  /* int idx = q->back - 1; */
  /* for(int i = 0; i < 10; i++) */
  /*   printf("%d\n", q->queue[idx = q_next_idx(q, idx)]->id); */

  Process* p = p_create(1, low_priority);
  printf("%s\n", p_to_string(p));

  return 0;
}
