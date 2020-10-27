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
		char buffer[1024];
		fgets(buffer, 1024, stdin); // Waits for user input into static buffer
		int receiveLen = strlen(buffer);

		// This while loop is only really a while loop
		// if no Nodes are ready for passing to sendThread
		// In which case writeThread waits to be signaled that there
		// is a free node then allocates again and tries to append message
		// again
		// This prevents memory leaks
		while(1) {
			char* message = (char*)malloc(receiveLen + 1);
			strcpy(message, buffer); // Copy already written message from user into dynamic
			if (Boss_appendList(out_list, message) == -1) {
				printf("Written message could not be added to out list buffer\n");
				printf("Attempting retry to send oldest written message\n");
				free(message); // Frees in case shutdown called during wait
				Boss_waitForNode(); // Waits in Boss to be told a node is free
			}
			else {
				pthread_mutex_lock(&out_mutex);
				{
					pthread_cond_signal(&out_cond); // Tells sendThread there's a new message
				}
				pthread_mutex_unlock(&out_mutex);
				break;
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

// Function used by List_clear in case there are still messages
// that need to be freed
void Write_freeMessages(void* message) {
	if (message) {
		free(message);
	}
}

// This is where sendThread will wait until it is signaled
// there is a message to be sent
// Written in write.c so mutexes and condition variables don't have
// to be passed back and forth
void Write_signalMsg(void) {
	pthread_mutex_lock(&out_mutex); 
	{
		pthread_cond_wait(&out_cond, &out_mutex);
	}
	pthread_mutex_unlock(&out_mutex);
}

void Write_shutdown(void){
	List_free(out_list, &Write_freeMessages);
	pthread_cancel(writeThread);
	pthread_join(writeThread, NULL);
	pthread_mutex_destroy(&out_mutex);
	pthread_cond_destroy(&out_cond);
}