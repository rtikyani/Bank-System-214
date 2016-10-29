#ifndef _MAIN_H_
#define _MAIN_H_

#include <pthread.h>
#include "account.h"

#define MAX_ACCOUNTS 20
#define SERVER_PORT 19945

typedef struct client {
    int file_descriptor;
    pthread_t thread;
    account_t *account;
} client_t;

#endif