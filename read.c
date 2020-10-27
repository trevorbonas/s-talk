#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "boss.h"
#include "list.h"
#include "receive.h"
#include "read.h"

List* in_list;

static pthread_t readThread;

void* readToScreen(void* unused){
	while(1){
		Receive_signalNewMsg(); // Waits for a message to be received
		const char* intro = "[Friend]: ";
		char* message = List_first(in_list); // Takes message from front of queue
		fputs(intro, stdout);
		fputs(message, stdout);
		fflush(stdout);
		if (*(message) == '!' && *(message + 2) == '\0') { // If receives a single '!' exits gracefully
			Boss_shutdown(); // receive.c will take care of freeing the message
		}
		else {
			free(message);
			Boss_removeNode(in_list);
		}
	}
}

void Read_init(List* list){
	in_list = list;
	pthread_create(&readThread, NULL, readToScreen, NULL);
}

void Read_shutdown(void) {
	pthread_cancel(readThread);
	pthread_join(readThread, NULL);
}