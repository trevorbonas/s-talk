#ifndef RECEIVE_H
#define RECEIVE_H
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

void* receiveTransmission(void* unused);
void Receive_freeMessages(void* message);
void Receive_signalNewMsg(void);
void Receive_init(List* list);
void Receive_shutdown(void);

#endif