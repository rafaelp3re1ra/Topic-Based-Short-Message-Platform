#include "topicManagement.h"
#include "userManagement.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

Topic topics[MAX_TOPICS];
int topics_count = 0;

int createTopic(const char *topicName)
{
    if (topics_count >= MAX_TOPICS)
    {
        printf("Erro: Número máximo de tópicos atingido!\n");
        return 1;
    }

    if (topicExists(topicName))
    {
        printf("Erro: Tópico já existe!\n");
        return 1;
    }

    strcpy(topics[topics_count].name, topicName);
    topics[topics_count].is_locked = 0;
    topics[topics_count].msg_count = 0;
    topics_count++;
    printf("Tópico '%s' criado com sucesso!\n", topicName);
    return 0;
}

void deleteTopic(const char *topicName)
{
    for (int i = 0; i < topics_count; i++)
    {
        if (strcmp(topics[i].name, topicName) == 0)
        {
            for (int j = i; j < topics_count - 1; j++)
            {
                topics[j] = topics[j + 1];
            }
            topics_count--;
            printf("Tópico '%s' eliminado com sucesso!\n", topicName);
            return;
        }
    }
    printf("Erro: Tópico '%s' não encontrado!\n", topicName);
}

void listTopics()
{
    if (topics_count == 0)
    {
        printf("Não existem topicos!\n");
        return;
    }
    for (int i = 0; i < topics_count; i++)
    {
        printf("Nome: %s - Mensagens Persistentes: %d - Estado: %d \n", topics[i].name, topics[i].msg_count, topics[i].is_locked);
    }
}

void showTopic(const char *topicName)
{
    if (topics_count == 0)
    {
        printf("Não existem topicos!\n");
        return;
    }

    for (int i = 0; i < topics_count; i++)
    {
        if (strcmp(topics[i].name, topicName) == 0)
        {
            printf("Topico: %s: \n", topicName);
            for (int j = 0; j < topics[i].msg_count; j++)
            {
                printf("%s - %s\n", topics[i].messages[j].sender, topics[i].messages[j].content);
            }
            return;
        }
    }
    printf("O topico %s não existe.\n", topicName);
}

void lockTopic(const char *topicName)
{
    if (topics_count == 0)
    {
        printf("Não existem topicos!\n");
        return;
    }

    for (int i = 0; i < topics_count; i++)
    {
        if (strcmp(topics[i].name, topicName) == 0)
        {
            if (topics[i].is_locked == 0)
            {
                topics[i].is_locked = 1;
                printf("Tópico %s bloqueado com sucesso!\n", topicName);
            }
            else
            {
                printf("O Tópico %s já está bloqueado!\n", topicName);
            }
            return; // Return para não mostrar a mensagem de tópico não existe
        }
    }
    printf("O Tópico %s não existe!\n", topicName);
}

void unlockTopic(const char *topicName)
{
    if (topics_count == 0)
    {
        printf("Não existem tópicos criados!\n");
        return;
    }

    for (int i = 0; i < topics_count; i++)
    {
        if (strcmp(topics[i].name, topicName) == 0)
        {
            if (topics[i].is_locked == 1)
            {
                topics[i].is_locked = 0;
                printf("Tópico %s desbloqueado com sucesso!\n", topicName);
            }
            else
            {
                printf("O Tópico %s já está desbloqueado!\n", topicName);
            }
            return; // Return para não mostrar a mensagem de tópico não existe
        }
    }
    printf("O Tópico %s não existe!\n", topicName);
}

int isTopicLocked(const char *topicName)
{
    for (int i = 0; i < topics_count; i++)
    {
        if (strcmp(topics[i].name, topicName) == 0)
        {
            return topics[i].is_locked;
        }
    }
    return 0;
}

int topicExists(const char *topicName)
{
    for (int i = 0; i < topics_count; i++)
    {
        if (strcmp(topics[i].name, topicName) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void addMessageToTopic(const char *topicName, const Message *msg)
{
    int topic_index = -1;
    for (int i = 0; i < topics_count; i++)
    {
        if (strcmp(topics[i].name, topicName) == 0)
        {
            topic_index = i;
            break;
        }
    }

    if (topic_index == -1)
    {
        if (!createTopic(topicName))
        {
            return;
        }
        topic_index = topics_count - 1;
    }

    if (topics[topic_index].is_locked)
    {
        printf("Erro: Tópico bloqueado!\n");
        return;
    }

    if (topics[topic_index].msg_count >= MAX_MESSAGES_PER_TOPIC)
    {
        // Se chegou ao limite, remove a mensagem mais antiga para dar lugar à nova
        for (int i = 0; i < topics[topic_index].msg_count - 1; i++)
        {
            topics[topic_index].messages[i] = topics[topic_index].messages[i + 1];
        }
        topics[topic_index].msg_count--;
    }

    topics[topic_index].messages[topics[topic_index].msg_count] = *msg;
    topics[topic_index].msg_count++;
}

void broadcastToSubscribers(const Message *msg)
{
    for (int i = 0; i < user_count; i++)
    {
        // Salta quem enviou a mensagem
        if (strcmp(users[i].userName, msg->sender) == 0)
            continue;

        for (int j = 0; j < users[i].sub_count; j++)
        {
            if (strcmp(users[i].subscriptions[j], msg->topic) == 0)
            {
                int fd = open(users[i].userPipe, O_WRONLY);
                if (fd >= 0)
                {
                    sendMessage(fd, msg);
                    close(fd);
                }
                break; // Só envia uma vez por utilizador
            }
        }
    }
}

void checkAndDeleteEmptyTopic(const char *topicName)
{
    if (!topicExists(topicName))
    {
        return;
    }

    int hasSubscribers = 0;
    int hasPersistentMessages = 0;
    int topicIndex = -1;

    for (int i = 0; i < topics_count; i++)
    {
        if (strcmp(topics[i].name, topicName) == 0)
        {
            topicIndex = i;
            hasPersistentMessages = (topics[i].msg_count > 0);
            break;
        }
    }

    if (topicIndex == -1)
        return;

    for (int i = 0; i < user_count && !hasSubscribers; i++)
    {
        for (int j = 0; j < users[i].sub_count; j++)
        {
            if (strcmp(users[i].subscriptions[j], topicName) == 0)
            {
                hasSubscribers = 1;
                break;
            }
        }
    }

    if (!hasSubscribers && !hasPersistentMessages)
    {
        deleteTopic(topicName);
    }
}

void removeExpiredMessage(int topicIndex, int messageIndex)
{
    for (int i = messageIndex; i < topics[topicIndex].msg_count - 1; i++)
    {
        topics[topicIndex].messages[i] = topics[topicIndex].messages[i + 1];
    }
    topics[topicIndex].msg_count--;

    if (topics[topicIndex].msg_count == 0)
    {
        checkAndDeleteEmptyTopic(topics[topicIndex].name);
    }
}

void *messageExpiration(void *arg)
{
    while (managerRunning)
    {
        for (int i = 0; i < topics_count; i++)
        {
            for (int j = 0; j < topics[i].msg_count; j++)
            {
                if (topics[i].messages[j].pers)
                {
                    topics[i].messages[j].duration--;
                    if (topics[i].messages[j].duration <= 0)
                    {
                        removeExpiredMessage(i, j);
                        j--;
                        if (i >= topics_count)
                            break;
                    }
                }
            }
        }
        sleep(1);
    }
    return NULL;
}

void notifyNewSubscriber(const char *username, const char *topicName)
{
    for (int i = 0; i < topics_count; i++)
    {
        if (strcmp(topics[i].name, topicName) == 0)
        {
            for (int j = 0; j < topics[i].msg_count; j++)
            {
                if (topics[i].messages[j].pers)
                {
                    Message persistentMsg = topics[i].messages[j];
                    persistentMsg.type = RESP_NORMAL;
                    sendResponseToUser(username, &persistentMsg);
                }
            }
            break;
        }
    }
}