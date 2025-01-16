#include "managerUtils.h"

int managerRunning = 1;

void handleCommand(const char *command)
{
    char cmd[50], arg1[100], arg2[100], arg3[100];

    sscanf(command, "%s %s %s %s", cmd, arg1, arg2, arg3);

    if (strcmp(cmd, "help") == 0)
    {
        printf("Comandos disponíveis:\n");
        printf(" - users (lista todos os utilizadores)\n");
        printf(" - remove <userName> (remove um utilizador)\n");
        printf(" - topics (lista todos os tópicos)\n");
        printf(" - lock <topic> (bloqueia um tópico)\n");
        printf(" - unlock <topic> (desbloqueia um tópico)\n");
        printf(" - show <topic> (mostra mensagens persistentes)\n");
        printf(" - close (encerra o manager)\n");
    }
    else if (strcmp(cmd, "users") == 0)
    {
        listUsers();
    }
    else if (strcmp(cmd, "remove") == 0)
    {
        removeUser(arg1, 1);
    }
    else if (strcmp(cmd, "topics") == 0)
    {
        listTopics();
    }
    else if (strcmp(cmd, "show") == 0)
    {
        showTopic(arg1);
    }
    else if (strcmp(cmd, "lock") == 0)
    {
        lockTopic(arg1);
    }
    else if (strcmp(cmd, "unlock") == 0)
    {
        unlockTopic(arg1);
    }
    else if (strcmp(cmd, "close") == 0)
    {
        shutdownManager();
    }
    else
    {
        printf("Comando desconhecido: %s\n", cmd);
    }
}

void *listenToFeeds(void *arg)
{
    const char *pipeGlobal = (const char *)arg;
    Message msg;
    int fd;

    fd = open(pipeGlobal, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("Erro ao abrir o pipe global");
        return NULL;
    }

    while (managerRunning == 1)
    {
        if (read(fd, &msg, sizeof(Message)) == sizeof(Message))
        {
            processMessage(&msg);
        }
    }

    close(fd);
    return NULL;
}

void listUsers()
{
    if (user_count == 0)
    {
        printf("Nenhum user ligado!\n");
        return;
    }

    printf("Users conectados (%d):\n", user_count);
    for (int i = 0; i < user_count; i++)
    {
        if (users[i].userName[0] != '\0')
        {
            printf(" - %s (PID: %d, Pipe: %s)\n",
                   users[i].userName, users[i].pid, users[i].userPipe);
        }
    }
    fflush(stdout);
}

void shutdownManager()
{
    printf("\nA encerrar o manager...\n");
    fflush(stdout);

    managerRunning = 0;

    char usernames[MAX_USERS][MAX_USERNAME];
    int count = 0;

    for (int i = 0; i < user_count; i++)
    {
        if (users[i].userName[0] != '\0')
        {
            strcpy(usernames[count], users[i].userName);
            count++;
        }
    }

    for (int i = 0; i < count; i++)
    {
        removeUser(usernames[i], 2);
        usleep(100000);
    }

    cleanupPipe(-1, "fifoGlobal");
    fflush(stdout);
}
