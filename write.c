#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "boss.h"
#include "write.h"
#include "list.h"

List* out_list;

static pthread_t writeThread;

static pthread_mutex_t out_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t out_cond = PTHREAD_COND_INITIALIZER;

void* writeMessage(void* unused) {
	while(1) {
		char* message = (char*)malloc(1024);
		message = fgets(message, 1024, stdin);
		if (message == NULL) {
			printf("ERROR: Message was not received from Keyboard\n");
			free(message);
		}
		else {
			if (Boss_appendList(out_list, message) == -1) {
				printf("ERROR: Keyboard buffer full, message not added to send queue\n");
				free(message);
			}
			else {
				pthread_mutex_lock(&out_mutex);
				{
					pthread_cond_signal(&out_cond);
				}
				pthread_mutex_unlock(&out_mutex);
			}

		}
	}

	return NULL;
}

void Write_init(List* list){
	out_list = list;
	pthread_t writeThread;
	pthread_create(&writeThread, NULL, writeMessage, NULL);
}

void Write_freeMessages(void* message) {
	if (message) {
		free(message);
		printf("Message freed!\n");
	}
}

void Write_signalMsg(void) {
	pthread_mutex_lock(&out_mutex); 
	{
		pthread_cond_wait(&out_cond, &out_mutex);
	}
	pthread_mutex_unlock(&out_mutex);
}

void Write_shutdown(void){
	pthread_cancel(writeThread);
	pthread_join(writeThread, NULL);
	List_free(out_list, &Write_freeMessages);
	pthread_mutex_destroy(&out_mutex);
	pthread_cond_destroy(&out_cond);
}