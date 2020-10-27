#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include "boss.h"
#include "receive.h"
#include "send.h"
#include "write.h"
#include "read.h"
#include "list.h"

int remote_port;
int local_port;
struct sockaddr_in remote_ip;

int main(int argc, char* argv[]) {
	// Argument for running program will be
	// ./s-talk (local port) (remote ip address) (remote port)

	if (argc != 4) {
		printf("Incorrect arguments: s-talk needs (user listening port) (remote ip address) (remote port)\n");
		return 1;
	}

	printf("Welcome to s-talk\nStart typing to chat\nType '!' to exit program\n\n");

	local_port = atoi(argv[1]);

	struct addrinfo *remoteAddress;
	struct addrinfo *current;

	int addr_check;
	struct  addrinfo hint;
	memset(&hint, 0, sizeof(struct addrinfo));
	// Setting for how I want input address to be interpreted
	hint.ai_flags = 0;
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_DGRAM;
	hint.ai_protocol = 0;
	hint.ai_addrlen = 0;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	addr_check = getaddrinfo(argv[2], argv[3], &hint, &remoteAddress);
	if (addr_check != 0) {
		printf("Remote ip as input is invalid\nExiting program\n");
		return 2;
	}

	// Data that will be received
	List* in = List_create();
	// Data to be sent out
	List* out = List_create();

	if (in == NULL || out == NULL) {
		printf("Lists could not be assigned\nExiting program\n");
		return 3;
	}

	Boss_addLocalPort(local_port); // Just means passing local_port only once

	// Pass the in list to both threads that will be using it
	Receive_init(in);
	Read_init(in);

	// Pass the out list to both threads that will be using it
	Write_init(out);
	Send_init(out, &remoteAddress);

	// Main thread will wait in Boss_exitSignal
	// until shutdown is signaled by one of the threads
	// in which case main thread will cancel and join all threads
	// free all memory not freed, clear the lists, destroy all
	// condition variables and mutexes, and finally return 0
	Boss_exitSignal();

	return 0;
}