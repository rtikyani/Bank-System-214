#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include "account.h"

account_t *create_account(char *name) {
    account_t *account = malloc(sizeof(account_t));
    account->name = strdup(name);
    account->active = false;
    account->balance = 0;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    if (pthread_mutex_init(&account->mutex, &attr)) {
        perror("Error could not create account mutex.");
        exit(EXIT_FAILURE);
    }

    return account;
}

void destroy_account(account_t *account) {
    free(account->name);
    free(account);
}

account_t *find_account(list_t *accounts, char *name) {
    list_iterator_t *iterator = create_iterator(accounts);

    account_t *account = get_item(iterator);

    while (account != NULL) {

        if (strcmp(account->name, name) == 0) {
            return account;
        }

        account = next_item(iterator);
    }

    destroy_iterator(iterator);

    return NULL;
}