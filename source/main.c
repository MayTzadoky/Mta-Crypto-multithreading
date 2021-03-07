#define _CRT_SECURE_NO_WARNINGS
#include <mta_rand.h>
#include <mta_crypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include "Queue.h"

void* ConsumersThreads(void* arg1);
void* serverThread(void* timeToWait);
int isStrPrint(char* str);
void getRandData(char* genratedPass,int passLength);

char* encryptedPass;
unsigned int passLength = 0;
int guessInTest = 1;
pthread_mutex_t g_lock;
pthread_cond_t g_swich_pass;
pthread_cond_t g_have_guess;
int res = 0;
struct Queue* queue;

int main(int argc, char** argv)
{
	int numOfDecrypters = 0;
	int timeout = 0;
	int resServer = 0;
	int res = 0;
	void* join_res;
	pthread_mutex_init(&g_lock, NULL);
	pthread_cond_init(&g_have_guess, NULL);
	pthread_cond_init(&g_swich_pass, NULL);

	if(argc < 5)
	{
		printf("Wrong arguments\n");
		exit(-1);
	}

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--password-length") == 0)
		{
			i++;
			passLength = atoi(argv[i]);
			if(passLength == 0)
			{
				printf("Password must be different from 0!\n");
				exit(-1);
			}
			if(passLength % 8 != 0)
			{
				printf("Password must be devided by 8!\n");
				exit(-1);
			}
			encryptedPass = (char*)malloc(sizeof(char) * 200 + 1 );
		}
		else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--num-of-decrypters") == 0)
		{
			i++;
			numOfDecrypters = atoi(argv[i]);
			if(numOfDecrypters == 0)
			{
				printf("The number of decrypters must be different from 0!\n");
				exit(-1);
			}
		}
		else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timeout") == 0)
		{
			i++;
			timeout = atoi(argv[i]);
			if(timeout == 0)
			{
				printf("The timeout must be different from 0!\n");
				exit(-1);
			}
		}
		else
		{
			printf("Wrong arguments\n");
			exit(-1);
		}
	}

	//server thread:
	pthread_t* pthread_id_server = malloc(sizeof(pthread_t));
	queue = createQueue();
	res = pthread_create(pthread_id_server, NULL, serverThread, &timeout);

	// client threads:
	//Allocate arrays whose size depends on user input
	pthread_t* pthread_id_arr = malloc(numOfDecrypters * sizeof(pthread_t));
	unsigned int* input_args = malloc(numOfDecrypters * (sizeof(unsigned int)));

	for(unsigned int i = 0; i < numOfDecrypters; i++)
	{
		input_args[i] = i;
		res = pthread_create(&pthread_id_arr[i], NULL, ConsumersThreads, &input_args[i]);
		assert(res == 0);
	}
	pthread_join(*pthread_id_server,NULL);

	free(encryptedPass);
	clear(queue);
	return 0;
}

void* serverThread(void* timeToWait)
{
	int keyLength = passLength/8;
	char* genratedPass = (char*)malloc(sizeof(char) * passLength + 1);
	char* genratedKey = (char*)malloc(sizeof(char) * keyLength + 1);
	printf("Thread #%lu created, Server Thread\n", pthread_self());
	struct timespec timeout;
	unsigned int encryptedPassLength = 0;
	int isPassGenerated = 0;	

	for(;;)
	{
		pthread_mutex_lock(&g_lock);
		if(queue->front == NULL && *(int*)timeToWait != 0 && isPassGenerated == 1) 
		{
			clock_gettime(CLOCK_REALTIME, &timeout);
			timeout.tv_sec += (*(int*)(timeToWait));

			res = pthread_cond_timedwait(&g_have_guess, &g_lock, &timeout);
			if (res == ETIMEDOUT && isPassGenerated != 0)
			{
				printf("#%lu\t[Server]\t[ERROR]\tNo password recieved during the configured timeout period (%d seconds), regenerating password\n",pthread_self(), *(int*)timeToWait);
				isPassGenerated = 0;
			}
		}

		if(queue->front == NULL && isPassGenerated == 1)
		{
			pthread_cond_wait(&g_have_guess, &g_lock);
		}
		
		if( queue->front != NULL && res != ETIMEDOUT && isPassGenerated ==1)
		{
			if(memcmp(genratedPass, queue->front->decryptedPass, queue->front->len)==0 && encryptedPassLength == queue->front->len)
			{
				printf("#%lu\t[Server]\t[OK]\tPassword decrypted successfully by client #%d, recieved %s, is %s\n",pthread_self(), queue->front->whoDecrypt, genratedPass, queue->front->decryptedPass);
				isPassGenerated = 0;
			}
			
			else
			{
				printf("#%lu\t[Server]\t[ERROR]\tWrong password received from client #%d(%s), should be (%s)\n",pthread_self(), queue->front->whoDecrypt, queue->front->decryptedPass,genratedPass);
			}

			deQueue(queue);
		}
			
		if (isPassGenerated == 0)
		{
			getRandData(genratedPass, passLength);
			MTA_get_rand_data(genratedKey, keyLength);

			MTA_encrypt(genratedKey, keyLength, genratedPass, passLength, encryptedPass, &encryptedPassLength);
			isPassGenerated = 1;
			printf("#%lu\t[Server]\t[INFO]\tNew password generated: %s, key: %s, after encryption: %s\n",pthread_self(), genratedPass, genratedKey,encryptedPass);
		}
		guessInTest = 0;
		pthread_mutex_unlock(&g_lock);
		pthread_cond_broadcast(&g_swich_pass);
	}
	free(genratedPass);
	free(genratedKey);
}

void* ConsumersThreads(void* arg1)
{
	printf("Thread #%lu created, input arg = %d\n", pthread_self(), *(int*)arg1);
	int keyLen = passLength/8;
	char* genratedKey = (char*)malloc(sizeof(char) * (keyLen+1));
	int decryptedLen = 0;
	char* encryptedPassword = (char*)malloc(sizeof(char) * 200);
	char* decryptedPassword = (char*)malloc(sizeof(char) * 200);
	int count = 0;
	for(;;)
	{
		pthread_mutex_lock(&g_lock);
		memcpy(encryptedPassword, encryptedPass, passLength);
		while(guessInTest == 1)
		{
			pthread_cond_wait(&g_swich_pass, &g_lock);
		}
		pthread_mutex_unlock(&g_lock);

		MTA_get_rand_data(genratedKey, keyLen);
		MTA_decrypt(genratedKey, keyLen, encryptedPassword, passLength, decryptedPassword, &decryptedLen);
		count++;
		if(isStrPrint(decryptedPassword) != 0)
		{
			pthread_mutex_lock(&g_lock);
			enQueue(queue,decryptedPassword,decryptedLen,*(int*)arg1, count);
			printf("#%lu\t[Client #%d]\t[INFO]\tAfter decryption: %s key guessed: %s, sending to server after %d iterations\n",pthread_self(),*(int*)arg1, decryptedPassword, genratedKey, count);
			guessInTest = 1;
			pthread_mutex_unlock(&g_lock);
			pthread_cond_broadcast(&g_have_guess);
			count = 0;
		}
	}

	free(genratedKey);
	free(decryptedPassword);
}

int isStrPrint(char* str)
{
	for(int i = 0; i < passLength; i++)
	{
		if((isprint(str[i]))==0)
		{
			return 0;
		}
	}
	return 1;
}

void getRandData(char* genratedPass,int passLength)
{
	char ch;
	for(int i=0; i<passLength; i++)
	{
		do
		{
			ch = MTA_get_rand_char();
		} while (!isprint(ch));
		
		genratedPass[i] = ch;
	}
}
