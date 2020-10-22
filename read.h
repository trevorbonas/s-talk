#ifndef READ_H
#define READ_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "boss.h"
#include "list.h"
#include "receive.h"
#include "read.h"

void* readToScreen(void* unused);
void Read_init(List* list);
void Read_shutdown(void);

#endif