#include "common.h"
#include "blog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFSZ 1024

void *receive_thread(void *arg)
{
    int *s_ptr = (int *)arg;
    int s = *s_ptr;

    struct BlogOperation operation;

    while (1)
    {

        size_t count = recv(s, &operation, sizeof(struct BlogOperation), 0);

        if (count == 0)
        {
            printf("Server disconnected.\n");
            close(s);
            exit(EXIT_FAILURE);
        }

        switch (operation.operation_type)
        {
        case 2:
            printf("New topic added in %s by %d:\n%s", operation.topic, operation.server_response, operation.content);
            break;

        case 3:
            printf("Available Topics:\n%s", operation.content);
            break;

        case 4:
            printf("Subscribed to topic: %s\n", operation.topic);
            break;

        case 5:
            printf("Connection closed.\n");
            close(s);
            exit(EXIT_SUCCESS);

        case 6:
            printf("Unsubscribed from topic: %s\n", operation.topic);
            break;

        case 8:
            printf("%s\n", operation.content);
            break;

        default:
            printf("Invalid operation type.\n");
            break;
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 3)
        usage(argc, argv);

    struct sockaddr_storage storage;
    if (0 != addrparse(argv[1], argv[2], &storage))
        usage(argc, argv);

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
        logexit("socket");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (connect(s, addr, sizeof(storage)) != 0)
        logexit("connect");

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("connected to %s\n", addrstr);

    struct BlogOperation operation;
    memset(&operation, 0, sizeof(struct BlogOperation));

    pthread_t recv_tid;

    if (pthread_create(&recv_tid, NULL, receive_thread, &s) != 0)
    {
        logexit("pthread_create");
    }

    while (1)
    {
        memset(&operation, 0, sizeof(struct BlogOperation));
        fgets(operation.content, sizeof(operation.content) - 1, stdin);
        operation.content[strlen(operation.content)-1] = '\0';

        setType(&operation, operation.content);

        if (operation.operation_type == 2)
        {
            printf("content> ");
            fgets(operation.content, sizeof(operation.content) - 1, stdin);
        }

        if (operation.operation_type == 5)
        {
            printf("Server disconnected.\n");
            close(s);
            exit(EXIT_FAILURE);
        }
        size_t count = send(s, &operation, sizeof(struct BlogOperation), 0);
        if (count != sizeof(struct BlogOperation))
        {
            logexit("send");
        }
    }

    if (pthread_join(recv_tid, NULL) != 0)
    {
        logexit("pthread_join");
    }

    close(s);
    printf("Connection closed.\n");

    exit(EXIT_SUCCESS);
}
