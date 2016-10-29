#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include "server.h"
#include "tokenizer.h"

list_t *clients;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

list_t *accounts;
pthread_mutex_t account_mutex = PTHREAD_MUTEX_INITIALIZER;

static void send_message(int file_descriptor, char *message) {
    write(file_descriptor, message, strlen(message) + 1);
}

void *handle_client(void *parameter) {
    client_t *client = parameter;
    int file_descriptor = client->file_descriptor;

    pthread_detach(pthread_self());

    send_message(file_descriptor, "\nServer Message:\tPlease enter a command:\n");

    char message[2048];

    while (read(file_descriptor, message, sizeof(message)) > 0) {

        if (strcmp(message, "0") == 0 || strcmp(message, "") == 0) {
            continue;
        }

        printf("Server received message: '%s'\n", message);

        tokenizer_t *tokenizer = create_tokenizer(message);

        char *command = next_token(tokenizer);

        if (command == NULL) {
            send_message(file_descriptor, "Message: Invalid syntax.\n");
            continue;
        }

        if (strcmp(command, "open") == 0) {

            if (get_size(accounts) == MAX_ACCOUNTS) {
                send_message(file_descriptor, "Message: Cannot create more than twenty accounts.\n");
                continue;
            }

            char *command_parameter = next_token(tokenizer);

            if (command_parameter == NULL) {
                send_message(file_descriptor, "Message: Invalid syntax.\n");
                continue;
            }

            if (find_account(accounts, command_parameter) != NULL) {
                send_message(file_descriptor, "Message: Cannot create account, name already in use.\n");
                continue;
            }

            account_t *account = create_account(command_parameter);

            pthread_mutex_lock(&account_mutex);
            insert_object(accounts, account);
            pthread_mutex_unlock(&account_mutex);

            send_message(file_descriptor, "Message: Successfully opened account.\n");

        } else if (strcmp(command, "start") == 0) {

            if (client->account != NULL && client->account->active) {
                send_message(file_descriptor, "Message: A session with an account is still in progress.\n");
                continue;
            }

            char *command_parameter = next_token(tokenizer);

            if (command_parameter == NULL) {
                send_message(file_descriptor, "Message: Invalid syntax.\n");
                continue;
            }

            account_t *account = find_account(accounts, command_parameter);

            if (account == NULL) {
                send_message(file_descriptor, "Message: Could not find an account with that name.\n");
                continue;
            } else if (account->active) {
                send_message(file_descriptor, "Message: Account is already in use.\n");
                continue;
            }

            while (pthread_mutex_trylock(&account->mutex) != 0) {
                printf("Could not lock already locked mutex.\n");
                send_message(file_descriptor, "Message: Waiting to start session for account.\n");
                sleep(1);
            }

            account->active = true;
            client->account = account;

            send_message(file_descriptor, "Message: Successfully connected to account.\n");

        } else if (strcmp(command, "credit") == 0) {

            if (client->account == NULL || !client->account->active) {
                send_message(file_descriptor, "Message: Not currently in a session.\n");
                continue;
            }

            char *command_parameter = next_token(tokenizer);

            if (command_parameter == NULL) {
                send_message(file_descriptor, "Message: Invalid syntax.\n");
                continue;
            }

            double amount = atof(command_parameter);

            if (amount <= 0) {
                send_message(file_descriptor, "Message: Not a valid credit amount.\n");
                continue;
            }

            double original_balance = client->account->balance;
            client->account->balance += amount;
            double balance = client->account->balance;

            char balance_message[2048];
            sprintf(balance_message, "Balance:\t$%.2lf\nCredit:\t$%.2lf\nNew Balance:\t$%.2lf\n", original_balance,
                    amount, balance);

            send_message(file_descriptor, balance_message);

        } else if (strcmp(command, "debit") == 0) {

            if (client->account == NULL || !client->account->active) {
                send_message(file_descriptor, "Message: Not currently in a session.\n");
                continue;
            }

            char *command_parameter = next_token(tokenizer);

            if (command_parameter == NULL) {
                send_message(file_descriptor, "Message: Invalid syntax.\n");
                continue;
            }

            double amount = atof(command_parameter);

            if (amount <= 0) {
                send_message(file_descriptor, "Message: Not a valid debit amount.\n");
                continue;
            }

            double original_balance = client->account->balance;
            client->account->balance -= amount;
            double balance = client->account->balance;

            char balance_message[2048];
            sprintf(balance_message, "Balance:\t$%.2lf\nWithdrawing:\t$%.2lf\nNew Balance:\t$%.2lf\n", original_balance,
                    amount, balance);
            send_message(file_descriptor, balance_message);

        } else if (strcmp(command, "balance") == 0) {

            if (client->account == NULL || !client->account->active) {
                send_message(file_descriptor, "Message: Not currently in a session.\n");
                continue;
            }

            double balance = client->account->balance;

            char balance_message[2048];
            sprintf(balance_message, "Balance:\t$%.2lf\n", balance);
            send_message(file_descriptor, balance_message);

        } else if (strcmp(command, "finish") == 0) {

            if (client->account == NULL || !client->account->active) {
                send_message(file_descriptor, "Message: Not currently in a session.\n");
                continue;
            }

            send_message(file_descriptor, "Message: Finishing account session.\n");

            pthread_mutex_unlock(&client->account->mutex);
            client->account->active = false;
            client->account = NULL;

        } else if (strcmp(command, "exit") == 0) {

            send_message(file_descriptor, "Message: Disconnecting from server.\n");
            destroy_tokenizer(tokenizer);
            break;
        }

        destroy_tokenizer(tokenizer);
    }

    if (client->account != NULL) {
        client->account->active = false;
        pthread_mutex_unlock(&client->account->mutex);
    }

    pthread_mutex_lock(&client_mutex);
    remove_object(clients, client);
    pthread_mutex_unlock(&client_mutex);

    close(file_descriptor);

    printf("Client disconnected.\n");

    return NULL;
}

static bool list_compare_function(void *first, void *second) {
    return true;
}

static void client_destroy_function(void *value) {
    client_t *client = value;

    if(client->account != NULL) {
        destroy_account(client->account);
    }

    free(client);
}

static void account_destroy_function(void *value) {
    destroy_account(value);
}

int main() {

    clients = create_list(&list_compare_function, &client_destroy_function);
    accounts = create_list(&list_compare_function, &account_destroy_function);

    int listen_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_socket;

    memset(&server_socket, '0', sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(SERVER_PORT);

    if (bind(listen_file_descriptor, (struct sockaddr *) &server_socket, sizeof(server_socket))) {
        perror("Error binding to port.");
        return EXIT_FAILURE;
    }

    if (listen(listen_file_descriptor, 100)) {
        perror("Error listening to port.");
        return EXIT_FAILURE;
    }

    while (true) {

        int file_descriptor = accept(listen_file_descriptor, NULL, NULL);
        if (file_descriptor == -1) {
            continue;
        }

        printf("Client connected.\n");

        client_t *client = malloc(sizeof(*client));
        client->file_descriptor = file_descriptor;
        client->account = NULL;

        pthread_mutex_lock(&client_mutex);
        insert_object(clients, client);
        pthread_mutex_unlock(&client_mutex);

        pthread_create(&client->thread, NULL, handle_client, client);

        sleep(1);
    }
}