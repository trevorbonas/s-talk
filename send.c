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
	printf("Send thread executing!\n");
	// Need to do a bunch of stuff here involving
	// mutexes and condition variables with write.c
	while(1) {
		Write_signalMsg();
		struct sockaddr_in sin = Boss_getSocket();
		int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
	
		char* message = List_first(out_list);
		List_remove(out_list);
		unsigned int sin_len = sizeof(sinRemote);

		if (sendto(socketDescriptor, message,
			sizeof(message), 0, (struct sockaddr*) &sinRemote,
			sin_len) == SO_ERROR) {
			printf("Message not successfully sent!\n");
		}
		// Has to check message + 2 since ENTER included '\n' and fgets
		// puts EOF after that
		if (*(message) == '!' && *(message + 2) == '\0') {
			Boss_shutdown();
		}
		free(message);
	}
}

void Send_init(List* list, struct sockaddr_in remoteAddress, int remotePort) {
	out_list = list;
	sinRemote = remoteAddress;
	remote_port = remotePort;

	printf("Remote address according to main arguments: %d\n", ntohs(sinRemote.sin_addr.s_addr));

	pthread_create(&sendThread, NULL, sendMessage, NULL);
}

void Send_shutdown(void) {
	printf("In send shutdown\n");
	pthread_cancel(sendThread);
	printf("Cancelled sendThread\n");
	pthread_join(sendThread, NULL);
	printf("Send shutdown finished\n");
}