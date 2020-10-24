#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
		int socketDescriptor = Boss_getSocket();
	
		char* message = List_first(out_list);
		List_remove(out_list);
		unsigned int sin_len = sizeof(sinRemote);

		int len = strlen(message);
		if (sendto(socketDescriptor, message,
			len + 1, 0, (struct sockaddr*) &sinRemote,
			sin_len) == SO_ERROR) {
			printf("ERROR: Message not successfully sent\n");
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
	memset(&sinRemote, 0, sizeof(sinRemote));
	sinRemote = remoteAddress;
	remote_port = remotePort;

	printf("Remote address according to main arguments: %s\n", inet_ntoa(sinRemote.sin_addr));
	printf("Remote port according to main arguments: %d\n", ntohs(sinRemote.sin_port));

	pthread_create(&sendThread, NULL, sendMessage, NULL);
}

void Send_shutdown(void) {
	pthread_cancel(sendThread);
	pthread_join(sendThread, NULL);
}