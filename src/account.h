#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_

#include <stdbool.h>
#include "list.h"

typedef struct account {
    pthread_mutex_t mutex;
    char *name;
    double balance;
    bool active;
} account_t;

account_t *create_account(char *);

void destroy_account(account_t *);

account_t *find_account(list_t *, char *);

#endif