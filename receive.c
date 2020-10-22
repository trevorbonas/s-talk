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
	printf("Receive thread executing!\n");
	// Address
	struct sockaddr_in sin = Boss_getSocket();

	printf("Local port according to Boss_getSocket: ");
	printf("%d\n", ntohs(sin.sin_port));

	int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

	while(1) {
		printf("Waiting for message\n");
		struct sockaddr_in sinRemote; // Address of sender
		unsigned int sin_len = sizeof(sinRemote);
		char* messageRx = (char*)malloc(1024); // Max length is 1024, 1024 characters
		int bytesRx = recvfrom(socketDescriptor,
			messageRx, sizeof(messageRx), 0, (struct sockaddr*) &sinRemote,
			&sin_len);

		int terminateIdx = (bytesRx < 1024)?bytesRx:1024 - 1;
		messageRx[terminateIdx] = 0;

		printf("Message received (%d bytes): \n\n'%s'\n", bytesRx, messageRx);

		Boss_appendList(in_list, messageRx);

		pthread_mutex_lock(&in_mutex);
		{
			pthread_cond_signal(&in_cond);
		}
		pthread_mutex_unlock(&in_mutex);
	}

}

void Receive_freeMessages(void* message) {
	free(message);
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
	printf("In receive shutdown\n");
	pthread_cancel(receiveThread);
	pthread_join(receiveThread, NULL);
	List_free(in_list, Receive_freeMessages); // Just in case there's something in buffer or read didn't free
}