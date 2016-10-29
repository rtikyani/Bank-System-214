#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include "client.h"

int file_descriptor;
pthread_t input_thread;
pthread_t output_thread;

static void send_message(int file_descriptor, char *message) {
    write(file_descriptor, message, strlen(message) + 1);
}

void *command_handler(void *parameter) {

    while (true) {

        char message[2048];
        scanf(" %[^\n]", message);

        send_message(file_descriptor, message);

        if (strcmp(message, "exit") == 0) {
            pthread_kill(input_thread, SIGINT);
            printf("Disconnected from server.");
            exit(EXIT_SUCCESS);
        }

        sleep(1);
    }
}

void *message_handler(void *parameter) {

    char message[2048];
    while (read(file_descriptor, message, sizeof(message)) > 0) {
        printf("%s", message);
    }

    return NULL;
}


void set_iaddr_str(struct sockaddr_in *sockaddr, char *address, uint16_t port) {
    struct hostent *host;

    memset(sockaddr, 0, sizeof(sockaddr));
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = htons(port);

    if ((host = gethostbyname(address)) == NULL) {
        printf("Could not get address information.\n");
    } else {
        bcopy(host->h_addr_list[0], &sockaddr->sin_addr, (size_t) host->h_length);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Error: Server host must be specified.");
        return EXIT_FAILURE;
    }

    char *address = argv[1];

    file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;

    set_iaddr_str(&server_address, address, SERVER_PORT);

    while (connect(file_descriptor, (const struct sockaddr *) &server_address, sizeof(server_address))) {
        printf("Connecting to %s...\n", address);
        perror("Could not connect to address.");

        sleep(3);
    }

    printf("Connected!\n");

    pthread_create(&input_thread, NULL, message_handler, address);
    pthread_detach(input_thread);

    pthread_create(&output_thread, NULL, command_handler, NULL);
    pthread_detach(input_thread);

    while (write(file_descriptor, "0", 2) == 2) {
        sleep(1);
    }

    pthread_kill(input_thread, SIGINT);
    pthread_kill(output_thread, SIGINT);

    close(file_descriptor);

    printf("Disconnected from server.\n");

    return EXIT_SUCCESS;
}
