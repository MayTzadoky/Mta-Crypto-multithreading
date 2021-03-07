#include "Queue.h"

struct QNode* newNode(char* Pass, int lenPass, unsigned int whoDecryptPass, int count) 
{ 
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode)); 
	temp->len = lenPass;
	temp->whoDecrypt = whoDecryptPass;
    temp->decryptedPass = (char*)malloc(sizeof(char)*(temp->len));
	memcpy(temp->decryptedPass, Pass, lenPass);
    temp->next = NULL; 
    temp->counter = count;
    return temp; 
} 
  
struct Queue* createQueue() 
{ 
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue)); 
    q->front = q->rear = NULL; 
    return q; 
} 
  
void enQueue(struct Queue* q, char* Pass, int lenPass, unsigned int whoDecryptPass, int count) 
{ 
    struct QNode* temp = newNode(Pass, lenPass, whoDecryptPass, count); 

    if (q->rear == NULL) 
	{ 
        q->front = q->rear = temp; 
        return; 
    } 
  
    q->rear->next = temp; 
    q->rear = temp; 
} 
  
void deQueue(struct Queue* q) 
{ 
    if (q->front == NULL) 
        return; 
  
    struct QNode* temp = q->front; 
  
    q->front = q->front->next; 
  
    if (q->front == NULL) 
        q->rear = NULL; 
  
    free(temp);
} 

void clear(struct Queue* q)
{
	while (q->front!=NULL)
	{
		deQueue(q);
	}
}