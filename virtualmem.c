#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <getopt.h>

#include "virtualmem.h"
#include "khash.h"

KHASH_MAP_INIT_INT(page_count, int)

Elem* initElem() {
  Elem* elem;
  elem = (Elem*)malloc(sizeof(Elem));
  if (elem == NULL) {
    perr("Cannot allocate the memory for the elem!\n");
  }

  elem->next = NULL;
  elem->prev = NULL;
  elem->count = 0;
  elem->age = 0;
  return elem;
}

Queue* initQueue(int maxSize) {
  Queue* Q;
  Q = (Queue*)malloc(sizeof(Queue));
  if (Q == NULL) {
    perr("Cannot allocate the memory for the queue!\n");
  }

  Q->size = 0;
  Q->front = NULL;
  Q->rear = NULL;
  Q->current = NULL;
  Q->maxSize = maxSize;
  return Q;
}

void traverse(Queue* Q) {
  Elem* current = initElem();
  current = Q->front;
  /*printf("frames: ");*/
  while (1) {
    if (current != NULL) {
      printf("val: %d, count: %d ; current: %d", current->val, current->count, Q->current->val);
      /*printf("%d, ", current->val);*/
      current = current->next;
    } else {
      break;
    }
  }
  printf("\n");
}

void increaseAge(Queue* Q) {
  Elem* current = initElem();
  current = Q->front;
  while (1) {
    if (current != NULL) {
      current->age += 1;
      current = current->next;
    } else {
      break;
    }
  }
}

Elem* search(Queue* Q, int val) {
  Elem* current = initElem();
  current = Q->front;
  for (current = Q->front; current; current = current->next) {
    if (current->val == val) {
      return current;
    }
  }
  return NULL;
}

int* importData(int* size, char* path) {
  FILE* file;
  char line[512];
  char* word;
  char* end;
  int num;
  int* data;
  int file_size;
  int i = 0;
  if (path != NULL) {
    file = fopen(path, "r");
    if (file == NULL) {
      perr("Cannot open the file!\n");
    } else {
      fseek(file, 0, SEEK_END);
      file_size = ftell(file);
      fseek(file, 0, SEEK_SET);
    }
    data = (int*)malloc((file_size/2 + 1) * sizeof(int));
  } else {
    printf("Please input the sequence: \n");
    file = stdin;
  }

  while(fgets(line, 512, file)) {
    if (line[0] == '\n') {
      continue;
    }
    word = strtok(line, " \t\n");
    num = strtol(word, &end, 10);
    if (!*end) {
      data[i] = num;
      i++;
    } else {
      perr("Input sequence error! \n");
    }
    while (1) {
      word = strtok(NULL, " \t\n");
      if(word == NULL) {
        break;
      } else {
        num = strtol(word, &end, 10);
        if (!*end) {
          data[i] = num;
          i++;
        } else {
          perr("Input sequence error!\n");
        }
      }
    }
    if (file == stdin) {
      break;
    }
  }
  *size = i;
  fclose(file);
  return &data[0];
}

void replace(Queue* Q, Elem* old, Elem* new) {
  if (old == Q->rear) {
    old->prev->next = new;
    new->prev = old->prev;
    Q->rear = new;
  } else if (old == Q->front) {
    old->next->prev = new;
    new->next = old->next;
    Q->front = new;
  } else {
    old->next->prev = new;
    new->prev = old->prev;
    old->prev->next = new;
    new->next = old->next;
  }
  free(old);
}

Elem* findVictim(Queue* Q, int flg) {
  Elem* current = Q->rear;
  int val = current->count;
  Elem* elem = current;
  while(current != NULL) {
    if ((val > current->count && flg == 1) || (val < current->count && flg == 0)) {
      val = current->count;
      elem = current;
    } else if (val == current->count) {
      if (elem->age < current->age) {
        elem = current;
      }
    }
    current = current->prev;
  }
  return elem;
}

void hitLRU_STACK(Queue* Q, Elem* e) {
  Elem* front = Q->front;
  Elem* ePrev = e->prev;
  Elem* eNext = e->next;
  if (e == Q->rear) {
    e->next = front;
    front->prev = e;
    ePrev->next = NULL;
    Q->rear = ePrev;
    Q->front = e;
  } else if (e == Q->front) {
    Q->front = front;
  } else {
    ePrev->next = eNext;
    eNext->prev = ePrev;
    e->next = front;
    front->prev = e;
    Q->front = e;
  }
}


void enqueueLRU_STACK(Queue* Q, int val) {
  Elem* old = initElem();
  Elem* new = initElem();
  new->val = val;
  old = Q->front;
  if (Q->size == 0) {
    Q->front = new;
    Q->rear = new;
    Q->size++;
  } else if (Q->size == Q->maxSize) {
    Elem* e = Q->rear;
    Q->rear = e->prev;
    Q->rear->next = NULL;
    free(e);
    old->prev = new;
    new->next = old;
    Q->front = new;
  } else {
    old->prev = new;
    new->next = old;
    Q->front = new;
    Q->size++;
  }
}

Elem* findPos_CLOCK(Queue* Q, Elem* current) {
  if (current == Q->front) {
    current = Q->rear;
  } else {
    current = current->prev;
  }
  while (current->count != 0) {
    current->count = 0;
    if (current == Q->front) {
      current = Q->rear;
    } else {
      current = current->prev;
    }
  }
  return current;
}

void enqueueLRU_CLOCK(Queue* Q, int val) {
  Elem* old = initElem();
  Elem* new = initElem();
  new->val = val;
  new->count = 0;
  old = Q->front;
  if (Q->size == 0) {
    Q->front = new;
    Q->rear = new;
    Q->current = new;
    Q->size++;
  } else if (Q->size == Q->maxSize) {
    Q->current = findPos_CLOCK(Q, Q->current);
    replace(Q, Q->current, new);
    Q->current = new;
  } else {
    old->prev = new;
    new->next = old;
    Q->front = new;
    Q->current = new;
    Q->size++;
  }
}


void countIncreaseLFU(khash_t(page_count)* h, int val) {
  khiter_t k;
  int isFound;
  k = kh_put(page_count, h, val, &isFound);
  if (isFound == 0) {
    kh_value(h, k) += 1;
  } else {
    kh_value(h, k) = 1;
  }
}

void enqueueLFU(Queue* Q, Queue* page_queue, int val, int count) {
  Elem* old = initElem();
  Elem* new = initElem();
  new->val = val;
  new->count = count;
  if (Q->size == 0) {
    Q->front = new;
    Q->rear = new;
    Q->size++;
  } else if (Q->size == Q->maxSize) {
    old = findVictim(Q, 1);
    replace(Q, old, new);
  } else {
    old = Q->front;
    old->prev = new;
    new->next = old;
    Q->front = new;
    Q->size++;
  }
  increaseAge(Q);
}

void enqueueFIFO(Queue* Q, int val) {
  Elem* old = initElem();
  Elem* new = initElem();
  new->val = val;
  old = Q->front;
  if (Q->size == 0) {
    Q->front = new;
    Q->rear = new;
    Q->size++;
  } else if (Q->size == Q->maxSize) {
    old = findVictim(Q, 1);
    replace(Q, old, new);
  } else {
    old->prev = new;
    new->next = old;
    Q->front = new;
    Q->size++;
  }
  increaseAge(Q);
}

void rightShiftLRU_REF8(Queue* Q) {
  Elem* current = initElem();
  current = Q->front;
  while (1) {
    if (current != NULL) {
      current->count = (current->count) >> 1;
      current = current->next;
    } else {
      break;
    }
  }
}

void enqueueLRU_REF8(Queue* Q, int val) {
  Elem* old = initElem();
  Elem* new = initElem();
  new->val = val;
  new->count = 128;
  if (Q->size == 0) {
    Q->front = new;
    Q->rear = new;
    Q->size++;
  } else if (Q->size == Q->maxSize) {
    rightShiftLRU_REF8(Q);
    old = findVictim(Q, 1);
    replace(Q, old, new);
  } else {
    rightShiftLRU_REF8(Q);
    old = Q->front;
    old->prev = new;
    new->next = old;
    Q->front = new;
    Q->size++;
  }
  increaseAge(Q);
}

int getFirAppPos(int* data, int startPos, int pageNum, int target) {
  int i;
  for (i = startPos; i < pageNum; i++) {
    if (target == data[i]) {
      return i;
    }
  }
  return pageNum;
}

Elem* findVictimOptimal(Queue* Q, int* data, int startPos, int pageNum) {
  Elem* current = Q->front;
  while (1) {
    if (current != NULL) {
      current->count = getFirAppPos(data, startPos, pageNum, current->val);
      current = current->next;
    } else {
      break;
    }
  }
  Elem* elem = findVictim(Q, 0);
  return elem;
}

void enqueueOptimal(Queue* Q, int* data, int startPos, int pageNum) {
  Elem* old = initElem();
  Elem* new = initElem();
  new->val = data[startPos];
  new->count = 0;
  if (Q->size == 0) {
    Q->front = new;
    Q->rear = new;
    Q->size++;
  } else if (Q->size == Q->maxSize) {
    old = findVictimOptimal(Q, data, startPos, pageNum);
    replace(Q, old, new);
  } else {
    old = Q->front;
    old->prev = new;
    new->next = old;
    Q->front = new;
    Q->size++;
  }
  increaseAge(Q);
}

int main(int argc, char *argv[]) {
  static const char help[] =
    "\nVM Manager Usage Summary:\n\n"
    " -h                    - Print this usage summary with all options.\n\n"
    " -f available-frames   - Set the number of available frames. Default is 5.\n\n"
    " -r replacement-policy - Set the page replacement policy. It can be either\n               FIFO (First-in-first-out)\n               LFU (Least-frequently-used)\n               LRU-STACK (Least-recently-used stack implementation)\n               LRU-CLOCK (Least-recently-used clock implementation)\n               LRU-REF8 (Least-recently-used Reference-bit implementation)\n\n"
    " -i input file         - Set the sequence data file. Default is read from STDIN. \n\n";

  int c;
  int frameSize = 5;
  char* path = NULL;
  int pageNum = 0;
  int* data;
  int i;
  int replaceCount = 0;
  int replaceCountOptimal = 0;
  struct timeval tv1, tv2;
  char* repAlg = NULL;

  opterr = 0;
  while((c = getopt(argc, argv, "hf:r:i:")) != -1) {
    switch(c) {
      case 'h':
        printf("%s", help);
        exit(0);
        break;
      case 'f':
        frameSize = atoi(optarg);
        break;
      case 'r':
        repAlg = optarg;
        break;
      case 'i':
        path = optarg;
        break;
      case '?':
        if (optopt == 'f') {
          perr("option -f needs an argument. \n");
        } else if (optopt == 'r') {
          perr("option -r needs an argument. \n");
        } else if (optopt == 'i') {
          perr("option -i needs an argument. \n");
        }
        return 1;
    }
  }
  data = importData(&pageNum, path);

  Queue* frames = initQueue(frameSize);
  int algFlg = 0;

  if (repAlg == NULL || strcasecmp(repAlg, "FIFO") == 0) {
    repAlg = "FIFO";
    algFlg = 1;
  } else if (strcasecmp(repAlg, "LFU") == 0) {
    algFlg = 2;
  } else if (strcasecmp(repAlg, "LRU-STACK") == 0) {
    algFlg = 3;
  } else if (strcasecmp(repAlg, "LRU-CLOCK") == 0) {
    algFlg = 4;
  } else if (strcasecmp(repAlg, "LRU-REF8") == 0) {
    algFlg = 5;
  } else {
    perr("Replacement algorithm input error! Please type -h to check the option!\n");
  }

  khash_t(page_count)* h = kh_init(page_count);
  Queue* pageQueue = initQueue(pageNum);
  khiter_t k;
  int hitCount;

  gettimeofday(&tv1, NULL);
  Elem *elem;
  for (i = 0; i < pageNum; i++) {
    if (algFlg == 2) {
      countIncreaseLFU(h, data[i]);
      k = kh_get(page_count, h, data[i]);
      hitCount = kh_value(h, k);
    }
    elem = search(frames, data[i]);
    if (elem == NULL) {
      if (frames->size == frames->maxSize) {
        replaceCount++;
      }
      switch (algFlg) {
        case 1:
          enqueueFIFO(frames, data[i]);
          break;
        case 2:
          enqueueLFU(frames, pageQueue, data[i], hitCount);
          break;
        case 3:
          enqueueLRU_STACK(frames, data[i]);
          break;
        case 4:
          enqueueLRU_CLOCK(frames, data[i]);
          break;
        case 5:
          enqueueLRU_REF8(frames, data[i]);
          break;
      }
    } else {
      switch (algFlg) {
        case 2:
          elem->count = hitCount;
          break;
        case 3:
          hitLRU_STACK(frames, elem);
          break;
        case 4:
          elem->count = 1;
          break;
        case 5:
          rightShiftLRU_REF8(frames);
          elem->count += 128;
          break;
      }
    }
    /*traverse(frames);*/
  }
  gettimeofday(&tv2, NULL);
  int exTime1 = ((tv2.tv_usec + 100000 * tv2.tv_sec) - (tv1.tv_usec + 100000 * tv1.tv_sec));


  gettimeofday(&tv1, NULL);
  frames = initQueue(frameSize);
  for (i = 0; i < pageNum; i++) {
    elem = search(frames, data[i]);
    if (elem == NULL) {
      if (frames->size == frames->maxSize) {
        replaceCountOptimal++;
      }
      enqueueOptimal(frames, data, i, pageNum);
    }
    /*traverse(frames);*/
  }
  gettimeofday(&tv2, NULL);
  int exTime2 = ((tv2.tv_usec + 100000 * tv2.tv_sec) - (tv1.tv_usec + 100000 * tv1.tv_sec));

  printf("# of page replacements with %s : %d\n", repAlg, replaceCount);
  printf("# of page replacements with Optimal : %d\n", replaceCountOptimal);
  float a = (float)replaceCount;
  float b = (float)replaceCountOptimal;
  float penalty;
  if (a == b) {
    penalty = 0;
  } else {
    penalty = ((a - b) / b) * 100;
  }
  printf("%% page replacement penalty using LFU: %0.1f%%\n", penalty);

  printf("\n");
  printf("Total time to run %s algorithm: %d msec\n", repAlg, exTime1);
  printf("Total time to run Optimal algorithm: %d msec\n", exTime2);
  float timePenalty = (((float)exTime2 - (float)exTime1) / (float)exTime2) * 100;
  if (timePenalty < 0) {
    printf("%s is %0.1f%% slower than Optimal algorithm.\n", repAlg, -timePenalty);
  } else {
    printf("%s is %0.1f%% faster than Optimal algorithm.\n", repAlg, timePenalty);
  }
  return 0;
}
