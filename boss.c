#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "boss.h"
#include "list.h"
#include "receive.h"
#include "send.h"
#include "write.h"
#include "read.h"

bool sockInit;
int local_port;
static pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t port_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t main_cond = PTHREAD_COND_INITIALIZER;

struct sockaddr_in sinNew;


// These are functions any thread can call

// Only main thread calls this, no need for anything fancy
// Prevents local port number from having to be passed to multiple threads
void Boss_addLocalPort(int x) {
	local_port = x;
}

struct sockaddr_in Boss_getSocket() {
	pthread_mutex_lock(&port_mutex);
	{
		if (!sockInit) {
			memset(&sinNew, 0, sizeof(sinNew));
			sinNew.sin_family = AF_INET;
			sinNew.sin_addr.s_addr = htonl(INADDR_ANY);
			sinNew.sin_port = htons(local_port);

			int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
			bind(socketDescriptor, (struct sockaddr*) &sinNew, sizeof(sinNew));
			sockInit = 1;
		}
	}
	pthread_mutex_unlock(&port_mutex);
	return sinNew;
}


// All nodes for lists come from a shared pool; this creates a race condition
// This function allows at most one thread to add a node to the list it is 
// working with, even when multiple threads are trying to at the same time
// There is no race condition with removing a node
int Boss_appendList(List* list, void* item) {
	int return_value = 0;
	pthread_mutex_lock(&list_mutex);
	{
		if (List_append(list, item) == -1) {
			return_value = -1;
		}
	}
	pthread_mutex_unlock(&list_mutex);

	return return_value;
}

// Calls shutdown for every thread
// Can be called from any thread
void Boss_shutdown(void) {
	//pthread_mutex_lock(&main_mutex);
	{
		pthread_cond_signal(&main_cond);
		printf("Just signaled main cond\n");
	}
	//pthread_mutex_unlock(&main_mutex);
}

void Boss_exitSignal(void) {
	pthread_mutex_lock(&main_mutex);
	{
		pthread_cond_wait(&main_cond, &main_mutex);
	}
	pthread_mutex_unlock(&main_mutex);
	
	Receive_shutdown();
	Send_shutdown();
	Write_shutdown();
	Read_shutdown();
	printf("Main thread now free to end!\n");
}