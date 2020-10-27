#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "boss.h"
#include "list.h"
#include "receive.h"

static pthread_t receiveThread;

static pthread_mutex_t in_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for 'in' list
static pthread_cond_t in_cond = PTHREAD_COND_INITIALIZER;	// Conditional variable for 'in' list
static List* in_list;

void* receiveTransmission(void* unused) {
	int socketDescriptor = Boss_getSocket();

	while(1) {
		struct sockaddr_in sinRemote; // Address of sender
		unsigned int sin_len = sizeof(sinRemote);
		char buffer[1024];
		int bytesRx = recvfrom(socketDescriptor,
			buffer, 1024, 0, (struct sockaddr*) &sinRemote,
			&sin_len);
		int terminateIdx = (bytesRx < 1024)?bytesRx:1024 - 1;
		buffer[terminateIdx] = '\0';
		int receiveLen = strlen(buffer);

		// The idea here is once we've received a message it goes into a buffer
		// Then memory is allocated according to this size of the received message
		// and s-talk attempts to add it to the in_list, for the screen to display the message
		// but if all nodes in the pool of static nodes are being used (unlikely if pool has 
		// number greater than 1) then the receive thread frees the memory and waits for a free node
		// in the pool. When a free node is available again it allocates memory again and tries to
		// append the message again. This prevents there being any memory leaks, say, for instance,
		// if the receive thread is waiting for a free node when the shutdown is called
		while(1) {
			char* messageRx = (char*)malloc(receiveLen); // Needs +1 because strlen doesn't could EOF
			strcpy(messageRx, buffer);

			if (Boss_appendList(in_list, messageRx) == -1) {
				printf("ERROR: Message could not be added to screen list buffer\n");
				printf("Attempting retry to display oldest received message\n");
				free(messageRx);
				Boss_waitForNode();
			}
			else {
				pthread_mutex_lock(&in_mutex);
				{
					pthread_cond_signal(&in_cond);
				}
				pthread_mutex_unlock(&in_mutex);
				break;
			}
		}
	}

	return NULL;
}

void Receive_freeMessages(void* message) {
	if (message) {
		free(message);
	}
}

void Receive_signalNewMsg(void){
	pthread_mutex_lock(&in_mutex); 
	{
		pthread_cond_wait(&in_cond, &in_mutex);
	}
	pthread_mutex_unlock(&in_mutex);
}

void Receive_init(List* list) {
	in_list = list;
	pthread_create(&receiveThread, NULL, receiveTransmission, NULL);
}

void Receive_shutdown(void) {
	List_free(in_list, &Receive_freeMessages); // Just in case there's something in buffer or read didn't free
	pthread_cancel(receiveThread);
	pthread_join(receiveThread, NULL);
	pthread_mutex_destroy(&in_mutex);
	pthread_cond_destroy(&in_cond);

}