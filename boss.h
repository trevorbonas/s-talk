#ifndef BOSS_H
#define BOSS_H
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "list.h"
#include "receive.h"
#include "send.h"
#include "write.h"
#include "read.h"

void Boss_addLocalPort(int x);
int Boss_getSocket();
int Boss_appendList(List* list, void* item);
void Boss_shutdown(void);
void Boss_exitSignal(void);

#endif