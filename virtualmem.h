#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

//#include "virtualmem.h"
//#include "khash.h"
#define perr(msg) { perror(msg); exit(1); }

//KHASH_MAP_INIT_INT(page_count, int);

typedef struct ElemStruct {
  struct ElemStruct* next;
  struct ElemStruct* prev;
  int val;
  int count;
  int age;
}Elem;

typedef struct QueueStruct {
  Elem* front;
  Elem* rear;
  Elem* current;
  int size;
  int maxSize;
}Queue;

Elem* initElem();

Queue* initQueue(int);

void traverse(Queue*);

void increaseAge(Queue*);

Elem* search(Queue*, int);

int* importData(int*, char*);

void replace(Queue*, Elem*, Elem*);

Elem* findVictim(Queue*, int);

void hitLRU_STACK(Queue*, Elem*);

void enqueueLRU_STACK(Queue*, int);

Elem* findPos_CLOCK(Queue*, Elem*);

void enqueueLRU_CLOCK(Queue*, int);

//void countIncreaseLFU(khash_t(page_count) *, int);

void enqueueLFU(Queue*, Queue*, int, int);

void enqueueFIFO(Queue*, int);

void rightShiftLRU_REF8(Queue*);

void enqueueLRU_REF8(Queue*, int);

int getFirAppPos(int*, int, int, int);

Elem* findVictimOptimal(Queue*, int*, int, int);

void enqueueOptimal(Queue*, int*, int, int);
