#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#define MAX_USERS 10

#include "../utils.h"
#include "topicManagement.h"
#include <pthread.h>

typedef struct
{
    char userName[50];
    int pid;
    char userPipe[50];
    char subscriptions[MAX_TOPICS][MAX_TOPIC_NAME];
    int sub_count;
} User;

extern User users[MAX_USERS];
extern int user_count;

User *findUser(const char *userName);
int userExists(const char *userName);
void addUser(const char *userName, int pid, const char *userPipe);
void removeUser(const char *userName, int bye);

void sendResponseToUser(const char *username, Message *response);
int subscribeUserToTopic(User *user, const char *topicName, int sendResponse);

void handleConnectMessage(const Message *msg);
void handleDisconnectMessage(const Message *msg);

void handleListMessage(const Message *msg);
void handleNormalMessage(const Message *msg);
void handleSubscribeMessage(const Message *msg);
void handleUnsubscribeMessage(const Message *msg);
void handleShowTopicMessages(const Message *msg);

void processMessage(const Message *msg);

int createPipe(const char *pipeName, int flags);
void cleanupPipe(int fd, const char *pipeName);

#endif