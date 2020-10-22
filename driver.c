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
	// ./s-talk (local port) (remote ip address) (remote number)

	if (argc != 4) {
		printf("Incorrect arguments: s-talk needs (user ip address) (remote ip address) (listening port)\n");
		return 1;
	}

	remote_port = atoi(argv[3]);
	local_port = atoi(argv[1]);
	memset(&remote_ip, 0, sizeof(remote_ip));
	remote_ip.sin_family = AF_INET;
	remote_ip.sin_port = htons(remote_port);
	inet_pton(AF_INET, argv[2], &remote_ip.sin_addr.s_addr);

	// After checks


	// Data that will be received
	List* in = List_create();
	// Data to be sent out
	List* out = List_create();

	Boss_addLocalPort(local_port);

	Receive_init(in);
	Read_init(in);

	Write_init(out);
	Send_init(out, remote_ip, remote_port);

	Boss_exitSignal();

	return 0;
}