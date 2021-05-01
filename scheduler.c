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
  int t;
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
  SchedulerEntry* entries[MAX_P];
  int size;
  int t;
} Scheduler;

SchedulerEntry* se_create(Process* p, int begin, int service_time,
                          int* io) {
  SchedulerEntry* se = (SchedulerEntry*) malloc(sizeof(SchedulerEntry));
  se->p = p;
  se->begin = begin;
  se->priority = high_priority;
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
  p->t = 0;
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

Scheduler* s_create(Queue* queues[3], SchedulerEntry** entries,
                    int size) {
  Scheduler* s = (Scheduler*) malloc(sizeof(Scheduler));
  if(queues == NULL){
    queues = (Queue**) malloc(sizeof(Queue*) * 3);
    for(int i = 0; i < 3; i++)
      queues[i] = q_create(NULL, 0);
  }
  s->t = 0;
  s->size = size;
  for(int i = 0; i < 3; i++)
    s->queues[i] = queues[i];
  for(int i = 0; i < size; i++)
    s->entries[i] = entries[i];
  return s;
}

int q_next_idx(Queue* q, int idx) {
  int i = idx;
  do {
    i = (i + 1) % (q->back);
  } while(q->queue[i] == NULL && i != q->front);
  return i;
}

char* q_to_string(Queue* q) {
  char* s = (char*) malloc(sizeof(char) * MAX_Q);
  int s_idx = 0;
  int i = q->front;
  s[s_idx++] = '[';
  bool st = true;
  if(q->size != 0) {
    do {
      if(!st) s[s_idx++] = ' ';
      s[s_idx++] = q->queue[i]->pid + '0';
      i = q_next_idx(q, i);
      st = false;
    } while(i != q->front);
  }
  s[s_idx++] = ']';
  s[s_idx] = '\0';
  return s;
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
  Process* p = q->queue[q->front];
  q->queue[q->front++] = NULL;
  return p;
}

void s_recieve_procs(Scheduler* s, int from) {
  for(int i = 0; i < s->size; i++)
    if(s->entries[i]->begin > from && s->entries[i]->begin <= s->t)
      q_push(s->queues[s->entries[i]->priority], s->entries[i]->p);
}

bool s_running(Scheduler* s) {
  for(int i = 0; i < 3; i++)
    if(s->queues[i]->size > 0) return true;
  return false;
}

SchedulerEntry* s_find_entry(Scheduler* s, Process* p) {
  for(int i = 0; i < s->size; i++)
    if(s->entries[i]->p == p) return s->entries[i];
  return NULL;
}

bool s_execute(Scheduler* s, int q_idx) {
  Process* p = q_pop(s->queues[q_idx]);
  SchedulerEntry* se = s_find_entry(s, p);
  if(se->priority < low_priority) se->priority++;

  // execute process
  for(int i = 0; i < QUANTUM; i++) {
    // check if io op now
    for(int j = 0; j < se->service_time; j++) {
      if(se->io[p->t+i]) {
        // TODO execute ios
        p->status = waiting;
      }
    }
    if(p->t + i <= se->service_time) p->t++;
    s->t++;
  }
  printf("scheduled proc: %5d, service time left: %3d\n\n",
         p->pid, se->service_time - p->t);

  // if done executing
  if(p->t == se->service_time) return true;

  // should't happen
  if(!q_push(s->queues[se->priority], p)) {
    printf("error: can't push to queue %d!\n", q_idx);
    return false;
  }
  return true;
}

void schedule(Scheduler* s) {
  int it = 0;
  do {
    bool executed = false;
    for(int i = 0; i < 3; i++) {
      s_recieve_procs(s, s->t-2);
      if(s->queues[i]->size == 0) continue;
      /* for(int j = 0; j < 3; j++) */
      /*   printf("%s\n", q_to_string(s->queues[j])); */
        printf("\nt: %8d,\n", s->t);
        printf("queues: \n\thigh: %s,\n\tnormal: %s,\n\tlow: %s\n\n",
                q_to_string(s->queues[high_priority]),
                q_to_string(s->queues[normal_priority]),
                q_to_string(s->queues[low_priority]));
      executed = s_execute(s, i);
      printf("\n");
      break;
    }
    /* if(it++ > 30) break; */
    /* s->t += 2; */
  } while(s_running(s));
}

void init() {
  PROC_COUNT = 1;
}

int main() {
  init();

  // TODO make a test case
  int ios_p1[5] = {0, 0, 0, 0, 0};
  SchedulerEntry* p1 = se_create(p_create(-1, -1, ready),
                                 0, 10, ios_p1);
  int ios_p2[5] = {0, 0, 0, 0, 0};
  SchedulerEntry* p2 = se_create(p_create(-1, -1, ready),
                                 2, 10, ios_p2);
  int ios_p3[5] = {0, 0, 0, 0, 0};
  SchedulerEntry* p3 = se_create(p_create(-1, -1, ready),
                                 5, 10, ios_p2);


  int n_entries = 3;
  SchedulerEntry* entries[3] = {p1, p2, p3};

  Scheduler* scheduler = s_create(NULL, entries, n_entries);

  schedule(scheduler);

  return 0;
}
