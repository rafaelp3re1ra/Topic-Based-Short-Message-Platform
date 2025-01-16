#ifndef TOPICMANAGEMENT_H
#define TOPICMANAGEMENT_H

#include "../utils.h"

extern int managerRunning;

#define MAX_MESSAGES_PER_TOPIC 100

typedef struct
{
    char name[MAX_TOPIC_NAME];
    int is_locked;
    int msg_count;
    Message messages[MAX_MESSAGES_PER_TOPIC];
} Topic;

extern Topic topics[MAX_TOPICS];
extern int topics_count;

int createTopic(const char *topicName);
void deleteTopic(const char *topicName);
void listTopics(void);
void showTopic(const char *topicName);
void lockTopic(const char *topicName);
void unlockTopic(const char *topicName);
int isTopicLocked(const char *topicName);
int topicExists(const char *topicName);

void addMessageToTopic(const char *topicName, const Message *msg);

void broadcastToSubscribers(const Message *msg);

void checkAndDeleteEmptyTopic(const char *topicName);

void removeExpiredMessage(int topicIndex, int messageIndex);

void *messageExpiration(void *arg);

void notifyNewSubscriber(const char *username, const char *topicName);

#endif
