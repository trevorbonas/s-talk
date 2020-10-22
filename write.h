#ifndef WRITE_H
#define WRITE_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "boss.h"
#include "write.h"
#include "list.h"

void* writeMessage(void* unused);
void Write_init(List* list);
void Write_freeMessages(void* message);
void Write_signalMsg(void);
void Write_shutdown(void);

#endif