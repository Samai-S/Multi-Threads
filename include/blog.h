#ifndef BLOG_H
#define BLOG_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_STRINGS 50
#define MAX_CLIENTS 100

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

void usage(int argc, char **argv);
int findTopicIndex(struct BlogOperation *operation, struct TopicId topicId[], int client_id[]);
int unsubscribeTopic(struct TopicId topicId[], struct BlogOperation *operation, int id);
void printTopics(struct TopicId topicId[], struct BlogOperation *operation);
int topicVerify(struct BlogOperation *operation, struct TopicId topicId[]);
int createOrSubscribeTopic(struct TopicId topicId[], char topic[], int client_id);
void swap(int *a, int *b);
int partition(int arr[], int low, int high);
void quicksort(int arr[], int low, int high);
void reverseQuicksort(int arr[], int size);
void setTopic(struct BlogOperation *operation, char *string);
int setType(struct BlogOperation *operation, char *string);
void initializeTopicId(struct TopicId topicId[]);

#endif
