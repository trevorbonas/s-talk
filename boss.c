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

// Boss takes care of allocation and deletion of nodes
// and binding a socket, as well as triggering complete shutdown
// something that wouldn't entirely be possible or clean
// if every thread had to manage in addition to what they
// already do

bool sockInit;
int local_port;

// Mutex for appending to list
static pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Mutex for removing a node from a list
static pthread_mutex_t remove_mutex = PTHREAD_MUTEX_INITIALIZER;

// Mutex for binding to a port
static pthread_mutex_t port_mutex = PTHREAD_MUTEX_INITIALIZER;

// Mutex and condition variable for node allocation and node availability signaling
static pthread_mutex_t node_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t node_cond = PTHREAD_COND_INITIALIZER;

// Mutex and condition variable for the main thread to wait on
static pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t main_cond = PTHREAD_COND_INITIALIZER;

struct sockaddr_in sinNew; // Socket for localhost
int socketDescriptor; // Socket descriptor returned to every thread that wants one


// All functions can be called by any thread

// Only main thread calls this, no need for anything fancy
// Prevents local port number from having to be passed in from multiple threads
void Boss_addLocalPort(int x) {
	local_port = x;
}

// Does the binding that allows the program to listen and send information with UDP
// Ensures that a socket is bound only once
int Boss_getSocket() {
	pthread_mutex_lock(&port_mutex);
	{
		if (!sockInit) {
			socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);

			memset(&sinNew, 0, sizeof(sinNew));
			sinNew.sin_family = AF_INET;
			sinNew.sin_addr.s_addr = htonl(INADDR_ANY);
			sinNew.sin_port = htons(local_port);

			if (bind(socketDescriptor, (struct sockaddr*) &sinNew, sizeof(sinNew)) < 0) {
				printf("ERROR: Socket could not be bound\nExiting program\n");
				Boss_shutdown();
			}
			sockInit = 1;
		}
	}
	pthread_mutex_unlock(&port_mutex);
	return socketDescriptor;
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

// There may be a race condition in removing nodes but the real
// purpose of this function is signalling a waiting thread that a node
// is available
void Boss_removeNode(List* list) {
	pthread_mutex_lock(&remove_mutex);
	{
		List_remove(list);
		pthread_cond_signal(&node_cond); // Signal a node is available
	}
	pthread_mutex_unlock(&remove_mutex);
}

// Signals main to shutdown and cleanup everything
// Can be called from any thread
void Boss_shutdown(void) {
	pthread_mutex_lock(&main_mutex);
	{
		pthread_cond_signal(&main_cond);
	}
	pthread_mutex_unlock(&main_mutex);
}

// Threads come here to wait for a signal that a new node is available
void Boss_waitForNode(void) {
	pthread_mutex_lock(&node_mutex);
	{
		pthread_cond_wait(&node_cond, &node_mutex);
	}
	pthread_mutex_unlock(&node_mutex);
}

// Complete shutdown by calling each thread's own shutdown function
// via main thread
void Boss_exitSignal(void) {
	pthread_mutex_lock(&main_mutex);
	{
		pthread_cond_wait(&main_cond, &main_mutex);
	}
	pthread_mutex_unlock(&main_mutex);

	printf("Program exiting\n");
	Send_shutdown();
	Read_shutdown();
	Write_shutdown();
	Receive_shutdown();
	pthread_mutex_destroy(&main_mutex);
	pthread_mutex_destroy(&list_mutex);
	pthread_mutex_destroy(&port_mutex);
	pthread_mutex_destroy(&node_mutex);
	pthread_mutex_destroy(&remove_mutex);
	pthread_cond_destroy(&main_cond);
	pthread_cond_destroy(&node_cond);
}