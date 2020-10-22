#ifndef SEND_H
#define SEND_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "boss.h"
#include "list.h"
#include "send.h"
#include "write.h"

void* sendMessage(void* unused);
void Send_init(List* list, struct sockaddr_in remoteAddress, int remotePort);
void Send_shutdown(void);

#endif