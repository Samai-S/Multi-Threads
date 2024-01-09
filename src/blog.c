#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_STRINGS 50
#define MAX_CLIENTS 100

void usage(int argc, char **argv)
{
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 8080\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct BlogOperation
{
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
};

struct TopicId
{
    char topic[50];
    int id[50];
    int id_count;
};

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high)
{
    int pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++)
    {
        if (arr[j] >= pivot)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quicksort(int arr[], int low, int high)
{
    if (low < high)
    {
        int pivotIndex = partition(arr, low, high);

        quicksort(arr, low, pivotIndex - 1);
        quicksort(arr, pivotIndex + 1, high);
    }
}

void reverseQuicksort(int arr[], int size)
{
    quicksort(arr, 0, size - 1);
}

int topicVerify(struct BlogOperation *operation, struct TopicId topicId[])
{
    for (int i = 0; i < 50; i++)
    {
        if (strcmp(topicId[i].topic, operation->topic) == 0)
        {
            return 1;
        }
    }

    return 0;
}

int findTopicIndex(struct BlogOperation *operation, struct TopicId topicId[], int client_id[])
{
    int index = -1;

    for (int i = 0; i < 50; i++)
    {
        if (strcmp(topicId[i].topic, operation->topic) == 0)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {

        for (int i = 0, j = 0; i < topicId[index].id_count; i++)
        {
            if (topicId[index].id[i] != -1)
            {
                client_id[j] = topicId[index].id[i];
                j++;
            }
        }
    }

    return index;
}

int unsubscribeTopic(struct TopicId topicId[], struct BlogOperation *operation, int id)
{
    int topicIndex = -1;

    for (int i = 0; i < 50; i++)
    {
        if (strcmp(topicId[i].topic, operation->topic) == 0)
        {
            topicIndex = i;

            break;
        }
    }

    if (topicIndex != -1)
    {

        for (int i = 0; i < 50; i++)
        {
            if (topicId[topicIndex].id[i] == id)
            {

                topicId[topicIndex].id[i] = -1;
                reverseQuicksort(topicId[topicIndex].id, 50);
                topicId[topicIndex].id_count = topicId[topicIndex].id_count - 1;
                return 1;
            }
        }
    }
    return 0;
}

void printTopics(struct TopicId topicId[], struct BlogOperation *operation)
{
    memset(operation->content, 0, sizeof(operation->content));

    for (int i = 0; i < 50; i++)
    {
        if (strlen(topicId[i].topic) > 0)
        {
            strncat(operation->content, topicId[i].topic, sizeof(operation->content) - strlen(operation->content) - 1);
            strncat(operation->content, "\n", sizeof(operation->content) - strlen(operation->content) - 1);
        }
    }
}

int createOrSubscribeTopic(struct TopicId topicId[], char topic[], int client_id)
{
    int topicIndex = -1;

    for (int i = 0; i < 50; i++)
    {
        if (strcmp(topicId[i].topic, topic) == 0)
        {
            topicIndex = i;
            break;
        }
    }

    if (topicIndex == -1)
    {
        for (int i = 0; i < 50; i++)
        {
            if (strlen(topicId[i].topic) == 0)
            {
                strncpy(topicId[i].topic, topic, sizeof(topicId[i].topic));
                topicId[i].id[0] = client_id;
                topicId[i].id_count = 1;
                break;
            }
        }
    }
    else
    {
        int verify = 0;
        for (int i = 0; i < 50; i++)
        {
            if (topicId[topicIndex].id[i] == client_id)
            {
                verify = 1;
                return 1;
            }
        }

        if (topicId[topicIndex].id_count < 50 && verify == 0)
        {
            topicId[topicIndex].id[topicId[topicIndex].id_count] = client_id;
            topicId[topicIndex].id_count++;
        }
    }
    return 0;
}

void setTopic(struct BlogOperation *operation, char *string)
{
    strcpy(operation->topic, string);
}

int setType(struct BlogOperation *operation, char *string)
{

    if (strncmp(string, "list topics", 11) == 0)
    {
        operation->operation_type = 3;
        return 1;
    }
    else if (strncmp(string, "subscribe", 9) == 0)
    {
        operation->operation_type = 4;
        setTopic(operation, string + 13);
        return 1;
    }
    else if (strncmp(string, "exit", 4) == 0)
    {
        operation->operation_type = 5;
        return 1;
    }
    else if (strncmp(string, "unsubscribe", 11) == 0)
    {
        operation->operation_type = 6;
        setTopic(operation, string + 15);
        return 1;
    }
    else if (strncmp(string, "publish", 7) == 0)
    {
        operation->operation_type = 2;
        setTopic(operation, string + 11);
        return 1;
    }
    else
    {

        operation->operation_type = -1;
        return 0;
    }
}

void initializeTopicId(struct TopicId topicId[])
{
    for (int i = 0; i < 50; i++)
    {
        memset(topicId[i].topic, 0, sizeof(topicId[i].topic));
        for (int j = 0; j < 50; j++)
        {
            topicId[i].id[j] = -1;
        }
    }
}