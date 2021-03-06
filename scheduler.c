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
  p->io = (int*) malloc(sizeof(int) * p->service_time);
  for(int i = 0; i < p->service_time; i++)
    p->io[i] = io[i];
  p->io = io;
  p->curr_io = 0;
  p->t_io = 0;
  return p;
}

void p_destroy(Process* p) {
  free(p);
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

void q_destroy(Queue* q) {
  free(q->queue);
  free(q);
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

void s_destroy(Scheduler* s) {
  for(int i = 0; i < s->size; i++)
    p_destroy(s->procs[i]);
  free(s->procs);
  for(int i = 0; i < 5; i++)
    q_destroy(s->queues[i]);
  free(s->queues);
  free(s);
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
    s_recieve_procs(s, s->t-QUANTUM);
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

Process* p_from_line(char* line) {
  const char* tok;
  int pid = -1, ppid = -1, begin, service_time;
  int* io;
  int field_idx = 0;

  char* c_ptr = line;
  char c;
  while((c = *c_ptr) != '\0') {
    int field_size = 0;
    char field[516];
    while(*(c_ptr+field_size) != '\0'
          && *(c_ptr+field_size) != '\n'
          && *(c_ptr+field_size) != '|') {
      field[field_size] = *(c_ptr+field_size);
      field_size++;
    }
    field[field_size] = '\0';

    if(field_size) {
      char io_s[128];
      int io_s_size = 0;
      int io_idx = 0;
      char* io_ptr = field;
      switch(field_idx) {
        // pid
        case 0:
          pid = atoi(field);
          break;
        // ppid
        case 1:
          ppid = atoi(field);
          break;
        // begin
        case 2:
          begin = atoi(field);
          break;
        // service_time
        case 3:
          service_time = atoi(field);
          io = (int*) malloc(sizeof(int) * service_time);
          break;
        // io
        case 4:
          while(*io_ptr != '\n'
                && *io_ptr != '\0') {
            io_s_size = 0;
            while(*io_ptr != ',' && *io_ptr != '\n' && *io_ptr != '\0') {
              io_s[io_s_size++] = *io_ptr++;
            }
            io_s[io_s_size] = '\0';
            io[io_idx++] = atoi(io_s);
            io_ptr++;
          }
          break;
        default:
          printf("input is not in the correct format!\n");
          exit(1);
      }
      field_idx++;
    }
    c_ptr += (field_size+1);
  }
  Process* p = p_create(pid, ppid, begin, service_time, io);
  return p;
}

Scheduler* s_from_csv(char* in_path) {
  Process* procs[MAX_P];
  FILE* f_stream = fopen(in_path, "r");
  char line[1024];
  int p_size = 0;
  while(fgets(line, 1024, f_stream)) {
    Process* p = p_from_line(line);
    procs[p_size++] = p;
  }
  fclose(f_stream);
  return s_create(procs, p_size);
}

void init() {
  PROC_COUNT = 1;
}

int main() {
  init();

  Scheduler* scheduler = s_from_csv("input.csv");

  schedule(scheduler);

  s_destroy(scheduler);
  return 0;
}
