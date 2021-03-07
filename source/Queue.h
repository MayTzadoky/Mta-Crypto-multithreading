#ifndef Queue_H_
#define Queue_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct QNode { 
    char* decryptedPass;
	int len;
	unsigned int whoDecrypt;
    int counter;
    struct QNode* next; 
}; 
  
struct Queue { 
    struct QNode *front, *rear; 
}; 

struct QNode* newNode(char* Pass, int lenPass, unsigned int whoDecryptPass, int count);
struct Queue* createQueue();
void enQueue(struct Queue* q, char* Pass, int lenPass, unsigned int whoDecryptPass, int count);
void deQueue(struct Queue* q);
void clear(struct Queue* q);

#endif
