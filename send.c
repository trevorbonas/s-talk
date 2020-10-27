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
struct addrinfo* sinRemote; // All address info for who to send to

void* sendMessage(void* unused) {
	while(1) {
		Write_signalMsg(); // sendThread waits until it is told a message is ready to be sent
		int socketDescriptor = Boss_getSocket();
	
		char* message = List_first(out_list); // Grab message from list

		int len = strlen(message);
		if (sendto(socketDescriptor, message,
			len + 1, 0, sinRemote->ai_addr,
			sinRemote->ai_addrlen) == -1) {
			printf("ERROR: Message not successfully sent\n");
		}
		// Has to check message + 2 since ENTER included '\n' and fgets
		// puts EOF after that
		if (*(message) == '!' && *(message + 2) == '\0') { // If user sendss a solo '!' gracefully exits
			Boss_shutdown(); // Any malloc'd message in list will be freed by shutdown
		}
		else {
			free(message);
			Boss_removeNode(out_list);
		}
	}
}

void Send_init(List* list, struct addrinfo** remoteAddress) {
	out_list = list;
	sinRemote = *remoteAddress;
	pthread_create(&sendThread, NULL, sendMessage, NULL);
}

void Send_shutdown(void) {
	pthread_cancel(sendThread);
	pthread_join(sendThread, NULL);
	free(sinRemote);
}