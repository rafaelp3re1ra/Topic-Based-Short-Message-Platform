#include "userManagement.h"

User users[MAX_USERS];
int user_count = 0;

User *findUser(const char *userName)
{
    User *found = NULL;
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].userName, userName) == 0)
        {
            found = &users[i];
            break;
        }
    }
    return found;
}

int userExists(const char *userName)
{
    int exists = 0;
    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].userName, userName) == 0)
        {
            exists = 1;
            break;
        }
    }
    return exists;
}

void addUser(const char *userName, int pid, const char *userPipe)
{
    if (user_count >= MAX_USERS)
    {
        return;
    }

    strcpy(users[user_count].userName, userName);
    users[user_count].pid = pid;
    strcpy(users[user_count].userPipe, userPipe);
    users[user_count].sub_count = 0;
    user_count++;

    printf("\nUser '%s' (PID: %d) a bordo!\n", userName, pid);
    fflush(stdout);
}

void handleConnectMessage(const Message *msg)
{
    Message response;
    strcpy(response.sender, "manager");

    // Pipe genérica para conexão
    char connPipe[50] = "tryConnect";
    snprintf(connPipe, sizeof(connPipe), "tryConnect_%d", msg->duration);

    if (mkfifo(connPipe, 0666) == -1)
    {
        printf("Erro ao criar pipe de conexão\n");
        return;
    }

    int fd = open(connPipe, O_WRONLY);

    if (userExists(msg->sender))
    {
        response.type = RESP_ERROR;
        strcpy(response.content, "Username já existe.");

        if (fd >= 0)
        {
            sendMessage(fd, &response);
            close(fd);
        }
        unlink(connPipe);
        return;
    }

    response.type = RESP_SUCCESS;
    strcpy(response.content, "Bem vindo!");
    sendMessage(fd, &response);
    close(fd);
    unlink(connPipe);

    char userPipe[50];
    snprintf(userPipe, sizeof(userPipe), "fifo_%s_%d", msg->sender, msg->duration);

    if (mkfifo(userPipe, 0666) == -1)
    {
        printf("Erro ao criar pipe para [%s]\n", msg->sender);
        return;
    }

    addUser(msg->sender, msg->duration, userPipe);
}

int subscribeUserToTopic(User *user, const char *topicName, int sendResponse)
{
    Message response;
    strcpy(response.sender, "manager");

    // Verifica se já está
    for (int i = 0; i < user->sub_count; i++)
    {
        if (strcmp(user->subscriptions[i], topicName) == 0)
        {
            if (sendResponse)
            {
                response.type = RESP_ERROR;
                snprintf(response.content, MAX_MSG_LEN, "Já está subscrito no tópico %s", topicName);
                sendResponseToUser(user->userName, &response);
            }
            return 0;
        }
    }

    // Aí vai ele
    strcpy(user->subscriptions[user->sub_count++], topicName);

    if (sendResponse)
    {
        response.type = RESP_SUCCESS;
        snprintf(response.content, MAX_MSG_LEN, "Subscrito no tópico %s com sucesso", topicName);
        sendResponseToUser(user->userName, &response);
        notifyNewSubscriber(user->userName, topicName);
    }
    return 1;
}

void removeUser(const char *userName, int bye)
{
    User *user = NULL;
    int userIndex = -1;

    for (int i = 0; i < user_count; i++)
    {
        if (strcmp(users[i].userName, userName) == 0)
        {
            user = &users[i];
            userIndex = i;
            break;
        }
    }

    if (!user)
    {
        printf("Erro: Utilizador '%s' não existe.\n", userName);
        return;
    }

    char userPipe[50];
    strcpy(userPipe, user->userPipe);
    char tempSubs[MAX_TOPICS][MAX_TOPIC_NAME];
    int tempCount = user->sub_count;
    for (int i = 0; i < tempCount; i++)
    {
        strcpy(tempSubs[i], user->subscriptions[i]);
    }

    if (bye > 0)
    {
        Message response;
        strcpy(response.sender, "manager");
        response.type = RESP_BYE_BYE;
        if (bye == 1)
            snprintf(response.content, MAX_MSG_LEN, "Removido pelo manager.");
        else
            snprintf(response.content, MAX_MSG_LEN, "Manager encerrou.");

        sendResponseToUser(userName, &response);
        printf("\nUtilizador '%s' desconectado.\n", userName);
    }

    usleep(100000);

    if (userIndex < user_count - 1)
    {
        users[userIndex] = users[user_count - 1];
    }
    user_count--;

    cleanupPipe(-1, userPipe);

    for (int i = 0; i < tempCount; i++)
    {
        checkAndDeleteEmptyTopic(tempSubs[i]);
    }

    printf("Utilizador '%s' removido.\n", userName);
}

void handleDisconnectMessage(const Message *msg)
{
    User *user = findUser(msg->sender);
    if (!user)
    {
        return;
    }

    printf("\nUtilizador '%s' desconectado.\n", msg->sender);
    fflush(stdout);
    removeUser(msg->sender, 0);
}

void sendResponseToUser(const char *username, Message *response)
{
    User *user = findUser(username);
    if (user)
    {
        int fd = open(user->userPipe, O_WRONLY);
        if (fd >= 0)
        {
            sendMessage(fd, response);
            close(fd);
        }
        else
        {
            printf("ERRO A RESPONDER AO USER");
        }
    }
}

void handleListMessage(const Message *msg)
{
    Message response;
    strcpy(response.sender, "manager");
    response.type = RESP_LIST;
    char buffer[MAX_MSG_LEN];

    if (topics_count == 0)
    {
        strcpy(buffer, "Nenhum topico criado!");
    }
    else
    {
        strcpy(buffer, "Tópicos disponíveis:\n");
        for (int i = 0; i < topics_count; i++)
        {
            char topicInfo[100];
            snprintf(topicInfo, sizeof(topicInfo),
                     "- %s (Msgs Persistentes: %d, Estado: %s)\n",
                     topics[i].name,
                     topics[i].msg_count,
                     topics[i].is_locked ? "Bloqueado" : "Desbloqueado");
            strcat(buffer, topicInfo);
        }
    }

    strcpy(response.content, buffer);
    sendResponseToUser(msg->sender, &response);
}

void handleNormalMessage(const Message *msg)
{
    User *sender = findUser(msg->sender);
    if (!sender)
    {
        return;
    }

    int topic_created = 0;
    if (!topicExists(msg->topic))
    {
        topic_created = createTopic(msg->topic);

        if (!topic_created)
        {
            subscribeUserToTopic(sender, msg->topic, 1);
        }
    }

    if (isTopicLocked(msg->topic))
    {
        Message response;
        strcpy(response.sender, "manager");
        response.type = RESP_ERROR;
        snprintf(response.content, MAX_MSG_LEN, "Erro: Tópico %s está bloqueado!", msg->topic);
        sendResponseToUser(msg->sender, &response);
        return;
    }

    broadcastToSubscribers(msg);

    if (msg->pers)
    {
        addMessageToTopic(msg->topic, msg);
    }
}

void handleSubscribeMessage(const Message *msg)
{
    Message response;
    strcpy(response.sender, "manager");

    User *user = findUser(msg->sender);

    if (!user)
    {
        return;
    }

    if (!topicExists(msg->topic))
    {
        response.type = RESP_ERROR;
        snprintf(response.content, MAX_MSG_LEN, "Tópico %s não existe", msg->topic);
        sendResponseToUser(msg->sender, &response);
        return;
    }

    subscribeUserToTopic(user, msg->topic, 1);
}

void handleUnsubscribeMessage(const Message *msg)
{
    Message response;
    strcpy(response.sender, "manager");

    User *user = findUser(msg->sender);
    if (!user)
    {
        return;
    }

    for (int i = 0; i < user->sub_count; i++)
    {
        if (strcmp(user->subscriptions[i], msg->topic) == 0)
        {
            char topicName[MAX_TOPIC_NAME];
            strcpy(topicName, user->subscriptions[i]);

            for (int j = i; j < user->sub_count - 1; j++)
            {
                strcpy(user->subscriptions[j], user->subscriptions[j + 1]);
            }
            user->sub_count--;

            response.type = RESP_SUCCESS;
            snprintf(response.content, MAX_MSG_LEN, "Cancelada subscrição do tópico %s", msg->topic);
            sendResponseToUser(msg->sender, &response);

            checkAndDeleteEmptyTopic(topicName);
            return;
        }
    }

    response.type = RESP_ERROR;
    snprintf(response.content, MAX_MSG_LEN, "Não está subscrito ao tópico %s", msg->topic);
    sendResponseToUser(msg->sender, &response);
}

void handleShowTopicMessages(const Message *msg)
{
    Message response;
    strcpy(response.sender, "manager");

    if (!topicExists(msg->topic))
    {
        response.type = RESP_ERROR;
        snprintf(response.content, MAX_MSG_LEN, "Tópico %s não existe", msg->topic);
    }
    else
    {
        response.type = RESP_LIST;
        char buffer[MAX_MSG_LEN] = "";

        snprintf(buffer, MAX_MSG_LEN, "=== Mensagens do Tópico: %s ===\n", msg->topic);
        int headerLen = strlen(buffer);
        int bufferSpace = MAX_MSG_LEN - headerLen;
        char *currentPos = buffer + headerLen;

        for (int i = 0; i < topics_count; i++)
        {
            if (strcmp(topics[i].name, msg->topic) == 0)
            {
                if (topics[i].msg_count == 0)
                {
                    snprintf(currentPos, bufferSpace, "Nenhuma mensagem persistente.");
                }
                else
                {
                    for (int j = 0; j < topics[i].msg_count && bufferSpace > 1; j++)
                    {
                        int written = snprintf(currentPos, bufferSpace, "%s: %s\n",
                                               topics[i].messages[j].sender,
                                               topics[i].messages[j].content);

                        if (written >= bufferSpace)
                            break;

                        currentPos += written;
                        bufferSpace -= written;
                    }
                }
                break;
            }
        }
        strncpy(response.content, buffer, MAX_MSG_LEN - 1);
        response.content[MAX_MSG_LEN - 1] = '\0';
    }
    sendResponseToUser(msg->sender, &response);
}

void processMessage(const Message *msg)
{
    if (strcmp(msg->content, "connect") == 0)
    {
        handleConnectMessage(msg);
        return;
    }
    if (strcmp(msg->content, "disconnect") == 0)
    {
        handleDisconnectMessage(msg);
        return;
    }

    switch (msg->type)
    {
    case MSG_NORMAL:
        handleNormalMessage(msg);
        break;
    case MSG_LIST:
        handleListMessage(msg);
        break;
    case MSG_SUBSCRIBE:
        handleSubscribeMessage(msg);
        break;
    case MSG_UNSUBSCRIBE:
        handleUnsubscribeMessage(msg);
        break;
    case MSG_SHOW:
        handleShowTopicMessages(msg);
        break;
    default:
    {
        Message response;
        strcpy(response.sender, "manager");
        response.type = RESP_ERROR;
        snprintf(response.content, MAX_MSG_LEN, "Tipo de mensagem desconhecida: %d", msg->type);
        sendResponseToUser(msg->sender, &response);
    }
    break;
    }
}

int createPipe(const char *pipeName, int flags)
{
    if (mkfifo(pipeName, 0666) == -1)
    {
        printf("Error creating pipe %s\n", pipeName);
        return -1;
    }
    return open(pipeName, flags);
}

void cleanupPipe(int fd, const char *pipeName)
{
    if (fd >= 0)
    {
        close(fd);
    }
    unlink(pipeName);
}