#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "boss.h"
#include "list.h"
#include "send.h"
#include "write.h"

// Network is big endian, for 0xa1cf 0xa1 is transmitted first (MSB first)

static pthread_t sendThread;

static List* out_list;

struct sockaddr_in sinRemote;
int remote_port;



void* sendMessage(void* unused) {
	// Need to do a bunch of stuff here involving
	// mutexes and condition variables with write.c
	while(1) {
		Write_signalMsg();
		struct sockaddr_in sin = Boss_getSocket();
		int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
	
		char* message = List_first(out_list);
		List_remove(out_list);
		unsigned int sin_len = sizeof(sinRemote);
		sendto(socketDescriptor, message,
			strlen(message), 0, (struct sockaddr*) &sinRemote,
			sin_len);

		if (*(message) == '!' && *(message + 1) == EOF) {
			Boss_shutdown();
		}
		free(message);
	}
}

void Send_init(List* list, struct sockaddr_in remoteAddress, int remotePort) {
	out_list = list;
	sinRemote = remoteAddress;
	remote_port = remotePort;
	pthread_create(&sendThread, NULL, sendMessage, NULL);
}

void Send_shutdown(void) {
	pthread_cancel(sendThread);
	pthread_join(sendThread, NULL);
}