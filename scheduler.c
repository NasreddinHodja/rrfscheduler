#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define MAX_Q 100
#define MAX_P 100
#define QUANTUM 2
#define MAX_IO_T 5
#define MAX_SERVICE_T 10

enum Q_TYPES {high_priority, low_priority, disk, mag_tape, printer};
enum PROC_STATUS {waiting, ready, running};

int PROC_COUNT;

typedef struct Process {
  int pid;
  int ppid;
  int status;
  int t;
  int begin;
  int priority;
  int service_time;
  int* io;
  int curr_io;
  int t_io;
} Process;

typedef struct Queue {
  Process** queue;
  int front;
  int back;
  int size;
} Queue;

typedef struct Scheduler {
  Process** procs;
  Queue** queues;
  int size;
  int t;
} Scheduler;

int gen_pid() {
  int pid = PROC_COUNT++;
  PROC_COUNT = PROC_COUNT % MAX_P;
  return pid;
}

int rand_duration(bool service_time) {
  if(service_time) return random() % (MAX_SERVICE_T) + 1;
  return random() % (MAX_IO_T) + 1;
}

Process* p_create(int pid, int ppid, int begin, int service_time,
                  int* io) {
  Process* p = (Process*) malloc(sizeof(Process));
  if(pid == -1) pid = gen_pid();
  if(ppid == -1) ppid = 0;
  p->pid = pid;
  p->ppid = 0;
  p->status = ready;
  p->t = 0;
  p->begin = begin;
  p->priority = high_priority;
  p->service_time = service_time;
  p->io = io;
  p->curr_io = 0;
  p->t_io = 0;
  return p;
}

char* p_to_string(Process* p) {
  char* p_info = (char*) malloc(100 * sizeof(char*));
  sprintf(p_info, "PID: %d, PPID: %d, status: %d",
          p->pid, p->ppid, p->status);

  char* s = (char*) malloc(sizeof(char) * 256);
  sprintf(s, "\nbegin: %d, service time: %d, priority: %d\n",
          p->begin, p->service_time, p->priority);
  strcat(p_info, s);

  char* io = (char*) malloc(sizeof(char) * 256);
  int io_idx = 0;
  io[io_idx++] = '[';
  io[io_idx++] = ' ';
  for(int i = 0; i < p->service_time; i++) {
    if(p->io[i] == -1)
      io[io_idx++] = '*';
    else
      io[io_idx++] = '0' + p->io[i];
    io[io_idx++] = ' ';
  }
  io[io_idx++] = ']';
  io[io_idx] = '\0';

  strcat(p_info, io);
  return p_info;
}

Queue* q_create(Process** procs, int size) {
  Queue* q = (Queue*) malloc(sizeof(Queue));
  q->front = 0;
  q->back = size;
  q->size = size;
  q->queue = (Process**) malloc(sizeof(Process*) * MAX_Q);
  if(procs == NULL) return q;
  for(int i = 0; i < size; i++)
    q->queue[i] = procs[i];
  return q;
}

Scheduler* s_create(Process** procs, int size) {
  Scheduler* s = (Scheduler*) malloc(sizeof(Scheduler));
  s->procs = (Process**) malloc(sizeof(Process*) * MAX_P);
  for(int i = 0; i < size; i++)
    s->procs[i] = procs[i];
  s->queues = (Queue**) malloc(sizeof(Queue*) * (printer+1));
  for(int i = high_priority; i <= printer; i++)
    s->queues[i] = q_create(NULL, 0);
  s->t = 0;
  s->size = size;
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
  char* s = (char*) malloc(sizeof(char) * (MAX_Q)*2);
  int s_idx = 0;
  int i = q->front;
  s[s_idx++] = '[';
  s[s_idx++] = ' ';
  if(q->size != 0) {
    do {
      s[s_idx++] = q->queue[i]->pid + '0';
      s[s_idx++] = ' ';
      i = q_next_idx(q, i);
    } while(i != q->front);
  }
  s[s_idx++] = ']';
  s[s_idx] = '\0';
  return s;
}

bool q_push(Queue* q, Process* p) {
  if(q->size == MAX_Q-1 || p == NULL) return false;
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
    if(s->procs[i]->begin > from && s->procs[i]->begin <= s->t)
      q_push(s->queues[s->procs[i]->priority], s->procs[i]);
}

bool s_running(Scheduler* s) {
  for(int i = high_priority; i <= printer; i++)
    if(s->queues[i]->size > 0) return true;
  return false;
}

void s_execute_io(Scheduler* s) {
  for(int i = disk; i <= printer; i++) {
    Queue* q = s->queues[i];
    if(q->size == 0) continue;
    Process* p = q_pop(q);
    p->t_io--;
    if(p->t_io == 0) {
      switch(p->curr_io) {
        case disk:
          p->priority = low_priority;
          q_push(s->queues[low_priority], p);
          break;
        case mag_tape:
        case printer:
          p->priority = high_priority;
          q_push(s->queues[high_priority], p);
          break;
      }
      p->curr_io = 0;
      continue;
    }
    q_push(q, p);
  }
}

void s_print(Scheduler* s) {
  printf("\nt: %8d\n", s->t);
  printf("queues: \n\thigh: %s\n\tlow: %s\nio:\n\tdisk: %s\n\tmag_tape: %s\n\tprinter: %s\n\n",
          q_to_string(s->queues[high_priority]),
          q_to_string(s->queues[low_priority]),
          q_to_string(s->queues[disk]),
          q_to_string(s->queues[mag_tape]),
          q_to_string(s->queues[printer]));
}

bool s_execute(Scheduler* s, int q_idx) {
  if(q_idx == -1) {
    s_print(s);
    for(int i = 0; i < QUANTUM; i++) {
      s_execute_io(s);
      s->t++;
    }
    printf("scheduled proc: none, service time left: none\n\n");
    return true;
  }

  s_print(s);
  // execute process
  Process* p = q_pop(s->queues[q_idx]);
  p->status = running;
  bool went_io = false;
  if(p->priority < low_priority) p->priority++;
  for(int i = 0; i < QUANTUM; i++) {
    if(p->curr_io != 0) {
      s_execute_io(s);
      continue;
    }
    // check if io op now
    if(p->io[p->t] != -1 && p->t < p->service_time) {
      p->status = waiting;
      p->t_io = rand_duration(false);
      p->curr_io = p->io[p->t];
      /* printf("===> %d goes to io for %d quantum\n", p->pid, p->t_io); */
      q_push(s->queues[p->io[p->t]], p);
      went_io = true;
      p->t++;
    }

    if(p->t + 1 <= p->service_time && p->curr_io == 0) p->t++;

    s_execute_io(s);
    s->t++;
  }
  printf("scheduled proc: %5d, service time left: %3d\n\n",
         p->pid, p->service_time - p->t);

  // if done executing
  if(p->t == p->service_time) return true;

  if(!went_io && p->curr_io == 0 && !q_push(s->queues[p->priority], p)) {
    // should't happen
    printf("error: can't push to queue %d!\n", q_idx);
    return false;
  }
  if(p->curr_io == 0) p->status = ready;
  return true;
}

void schedule(Scheduler* s) {
  bool st = true;
  do {
    bool executed = false;
    if(st) printf("------------//--------------\n");
    s_recieve_procs(s, s->t-2);
    for(int i = high_priority; i <= low_priority; i++) {
      if(s->queues[i]->size == 0) continue;
      executed = s_execute(s, i);
      break;
    }
    if(executed == false) {
      executed = s_execute(s, -1);
    }
    printf("------------//--------------\n");
    st = false;
  } while(s_running(s));

  printf("Done!\n");
}

void init() {
  PROC_COUNT = 1;
}

int main() {
  init();

  // TODO make a test case
  int ios_p1[5] = {-1, printer, -1, -1, -1};
  Process* p1 = p_create(-1, -1, 0, 5, ios_p1);
  int ios_p2[5] = {-1, printer, -1, -1, -1};
  Process* p2 = p_create(-1, -1, 2, 5, ios_p2);
  int ios_p3[5] = {-1, -1, -1, mag_tape, -1};
  Process* p3 = p_create(-1, -1, 1, 5, ios_p3);

  Process* procs[3] = {p1, p2, p3};

  Scheduler* scheduler = s_create(procs, 3);

  schedule(scheduler);

  return 0;
}
