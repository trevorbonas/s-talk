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
		Receive_signalNewMsg();
		const char* intro = "friend: ";
		const char* nl = "\n";
		char* message = List_first(in_list);
		List_remove(in_list);
		fputs(intro, stdout);
		fputs(message, stdout);
		fputs(nl, stdout);
		if (*(message) == '!' && *(message + 1) == EOF) {
			Boss_shutdown(); // receive.c will take care of freeing the message
		}
		free(message);
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