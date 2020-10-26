#ifndef SEND_H
#define SEND_H
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

void* sendMessage(void* unused);
void Send_init(List* list, struct addrinfo** remoteAddress);
void Send_shutdown(void);

#endif