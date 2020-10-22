#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <netdb.h>
#include "list.h"

int receive(pthread_t* thread, List* list);

int send(pthread_t* thread, List* list);

int screen_out(pthread_t* thread, List* list);

int keyboard_in(pthread_t* thread, List* list);