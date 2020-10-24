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

	struct addrinfo *returned_info;
	int addr_check;
	struct  addrinfo hint;
	memset(&hint, 0, sizeof(struct addrinfo));
	hint.ai_flags = AI_PASSIVE;
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_DGRAM;
	hint.ai_protocol = 0;
	hint.ai_addrlen = 0;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	/*addr_check = getaddrinfo(argv[2], argv[3], &hint, &returned_info);
	if (addr_check != 0) {
		printf("Remote ip as input is invalid\nExiting program\n");
		return 2;
	}
	remote_ip.sin_addr = returned_info->ai_addr; */

	struct hostent* hostinfo = gethostbyname(argv[2]);
	bcopy((char*)hostinfo->h_addr, (char*)&remote_ip.sin_addr, hostinfo->h_length);

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