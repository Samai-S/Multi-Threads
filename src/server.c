#include "common.h"
#include "blog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024
#define MAX_CLIENTS 100

struct TopicId topicId[50];

struct ClientData
{
    int csock;
    struct sockaddr_storage storage;
    int client_id;
};

struct ClientData clients[MAX_CLIENTS];
int next_client_id = 1;
pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void sendToSubscribedClients(struct BlogOperation *operation, int client_id, struct TopicId topicId[])
{
    pthread_mutex_lock(&clients_mutex);

    int clients_id[MAX_CLIENTS];
    int topicIndex = findTopicIndex(operation, topicId, clients_id);
    for (int i = 0; i < topicId[topicIndex].id_count; i++)
    {
        if (clients[clients_id[i]].csock != -1)
        {
            struct BlogOperation clone_operation;
            memcpy(&clone_operation, operation, sizeof(struct BlogOperation));

            send(clients[clients_id[i]].csock, &clone_operation, sizeof(struct BlogOperation), 0);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void *client_thread(void *data)
{
    struct ClientData *cdata = (struct ClientData *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] Connection from %s\n", caddrstr);

    struct BlogOperation operation;

    pthread_mutex_lock(&id_mutex);
    cdata->client_id = next_client_id;
    next_client_id++;
    pthread_mutex_unlock(&id_mutex);
    clients[next_client_id - 1] = *cdata;

    while (1)
    {
        memset(&operation, 0, sizeof(struct BlogOperation));
        size_t count = recv(cdata->csock, &operation, sizeof(struct BlogOperation), 0);

        if (count <= 0)
        {
            printf("[log] Client %d disconnected\n", cdata->client_id);
            break;
        }

        if (operation.operation_type == 2)
        {
            if (!topicVerify(&operation, topicId) || strcmp(operation.topic, "") == 0)
            {
                strcpy(operation.content, "error: Invalid topic\n");
                operation.operation_type = 8;
                send(cdata->csock, &operation, sizeof(struct BlogOperation), 0);
            }
            else
            {
                operation.server_response = cdata->client_id;
                sendToSubscribedClients(&operation, cdata->client_id, topicId);
            }
        }
        else if (operation.operation_type == 3)
        {
            printTopics(topicId, &operation);
            send(cdata->csock, &operation, sizeof(struct BlogOperation), 0);
        }
        else if (operation.operation_type == 4)
        {
            if (strcmp(operation.topic, "") == 0)
            {
                strcpy(operation.content, "Invalid topic\n");
                operation.operation_type = 8;
                send(cdata->csock, &operation, sizeof(struct BlogOperation), 0);
            }
            else if (!createOrSubscribeTopic(topicId, operation.topic, cdata->client_id))
            {
                send(cdata->csock, &operation, sizeof(struct BlogOperation), 0);
            }
            else
            {
                strcpy(operation.content, "error: Already subscribed");
                operation.operation_type = 8;
                send(cdata->csock, &operation, sizeof(struct BlogOperation), 0);
            }
        }
        else if (operation.operation_type == 6)
        {
            if (!topicVerify(&operation, topicId) || strcmp(operation.topic, "") == 0)
            {
                strcpy(operation.content, "error: Invalid topic\n");
                operation.operation_type = 8;
                send(cdata->csock, &operation, sizeof(struct BlogOperation), 0);
            }
            else if (unsubscribeTopic(topicId, &operation, cdata->client_id))
            {
                send(cdata->csock, &operation, sizeof(struct BlogOperation), 0);
            }
            else
            {
                strcpy(operation.content, "error: You are no subscribed at this topic");
                operation.operation_type = 8;
                send(cdata->csock, &operation, sizeof(struct BlogOperation), 0);
            }
        }
    }

    close(cdata->csock);
    free(cdata);

    return NULL;
}

int main(int argc, char **argv)
{
    initializeTopicId(topicId);

    if (argc < 3)
        usage(argc, argv);

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
        usage(argc, argv);

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
        logexit("socket");

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
        logexit("setsockopt");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (bind(s, addr, sizeof(storage)) != 0)
        logexit("bind");

    if (listen(s, 10) != 0)
        logexit("listen");

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("Bound in %s, waiting for connections\n", addrstr);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].csock = -1;
    }

    while (1)
    {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1)
            logexit("accept");

        struct ClientData *cdata = malloc(sizeof(struct ClientData));
        if (!cdata)
            logexit("malloc");

        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
    }

    exit(EXIT_SUCCESS);
}