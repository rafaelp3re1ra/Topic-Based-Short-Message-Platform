#include "feedUtils.h"

int feedRunning = 1;

void initFeed(FeedContext *ctx, const char *user)
{
    ctx->running = 0;
    int pid = getpid();

    strncpy(ctx->username, user, MAX_USERNAME - 1);
    ctx->username[MAX_USERNAME - 1] = '\0';

    snprintf(ctx->pipe_name, sizeof(ctx->pipe_name), "fifo_%s_%d", user, pid);

    ctx->managerPipe = open("fifoGlobal", O_WRONLY);
    if (ctx->managerPipe < 0)
    {
        printf("Erro: O manager está a dormir.\n");
        return;
    }

    Message msg;
    strcpy(msg.sender, ctx->username);
    strcpy(msg.content, "connect");
    msg.duration = pid;

    if (sendMessage(ctx->managerPipe, &msg) <= 0)
    {
        printf("Erro ao enviar pedido de conexão.\n");
        close(ctx->managerPipe);
        return;
    }

    usleep(100000); // Pequena espera

    // Tentar conexão
    char connPipe[50] = "tryConnect";
    int fd = open(connPipe, O_RDONLY);
    if (fd < 0)
    {
        printf("Erro ao abrir pipe de resposta!\n");
        close(ctx->managerPipe);
        return;
    }

    Message response;
    if (read(fd, &response, sizeof(Message)) > 0)
    {
        close(fd);
        if (response.type == RESP_SUCCESS)
        {
            ctx->running = 1;
            printf("Mensagem do manager: %s\n", response.content);
            return;
        }
        printf("Erro na conexão: %s\n", response.content);
        close(ctx->managerPipe);
        return;
    }
    close(fd);
    close(ctx->managerPipe);
}

void handleCommandResponse(Message *msg)
{
    switch (msg->type)
    {
    case RESP_LIST:
        printf("\n%s\n", msg->content);
        break;
    case RESP_ERROR:
        printf("\n[ERRO] %s\n", msg->content);
        break;
    case RESP_SUCCESS:
        printf("\n[SUCESSO] %s\n", msg->content);
        break;
    default:
        printf("\n[Resposta %d] %s\n", msg->type, msg->content);
    }
    fflush(stdout);
}

void handle_input(FeedContext *ctx)
{
    printf("'help' para ver comandos:\n-> ");
    char buffer[256];
    if (readInput(buffer, sizeof(buffer)) <= 0)
    {
        return;
    }

    Message msg;
    strcpy(msg.sender, ctx->username);

    char cmd[50], arg1[50], arg2[50], content[MAX_MSG_LEN];

    if (strcmp(buffer, "help") == 0)
    {
        printf("Comandos:\n"
               "list - listar tópicos\n"
               "show <topico> - ver mensagens\n"
               "subscribe <topico> - subscrever\n"
               "unsubscribe <topico> - cancelar subscrição\n"
               "msg <topico> <duração> <mensagem> - enviar mensagem\n"
               "close - sair\n");
        return;
    }
    else if (strcmp(buffer, "close") == 0)
    {
        shutdownFeed(ctx, 0);
        return;
    }
    else if (strcmp(buffer, "list") == 0)
    {
        msg.type = MSG_LIST;
    }
    else if (sscanf(buffer, "show %s", arg1) == 1)
    {
        msg.type = MSG_SHOW;
        strcpy(msg.topic, arg1);
    }
    else if (sscanf(buffer, "subscribe %s", arg1) == 1)
    {
        msg.type = MSG_SUBSCRIBE;
        strcpy(msg.topic, arg1);
    }
    else if (sscanf(buffer, "unsubscribe %s", arg1) == 1)
    {
        msg.type = MSG_UNSUBSCRIBE;
        strcpy(msg.topic, arg1);
    }
    else if (sscanf(buffer, "msg %s %s %[^\n]", arg1, arg2, content) == 3)
    {
        msg.type = MSG_NORMAL;
        strcpy(msg.topic, arg1);
        msg.duration = atoi(arg2);
        strcpy(msg.content, content);
        msg.pers = (msg.duration > 0);
    }
    else if (!ctx->running)
    {
        printf("Feed encerrado.");
        return;
    }
    else
    {
        printf("Comando inválido\n");
        return;
    }

    sendMessage(ctx->managerPipe, &msg);
}

void shutdownFeed(FeedContext *ctx, int bye)
{
    ctx->running = 0;

    if (!bye)
    {
        Message msg;
        strcpy(msg.sender, ctx->username);
        strcpy(msg.content, "disconnect");
        sendMessage(ctx->managerPipe, &msg);
    }

    if (ctx->managerPipe > 0)
        close(ctx->managerPipe);
    if (ctx->responsePipe > 0)
        close(ctx->responsePipe);

    printf("\nA encerrar...\n");
    printf("Pressione ENTER para sair.\n");
    fflush(stdout);
}

void *listen_manager(void *arg)
{
    FeedContext *ctx = (FeedContext *)arg;
    Message msg;

    if (!ctx->running)
    {
        return NULL;
    }

    ctx->responsePipe = open(ctx->pipe_name, O_RDONLY | O_NONBLOCK);
    if (ctx->responsePipe < 0)
    {
        printf("Erro ao abrir pipe de resposta!\n");
        return NULL;
    }

    while (ctx->running)
    {
        if (read(ctx->responsePipe, &msg, sizeof(Message)) > 0)
        {
            if (msg.type == RESP_BYE_BYE)
            {
                printf("\n[BYE BYE] %s\n", msg.content);
                shutdownFeed(ctx, 1);
                break;
            }
            else if (msg.type == RESP_NORMAL)
            {
                printf("\n%s: %s\n-> ", msg.sender, msg.content);
                fflush(stdout);
            }
            else
            {
                handleCommandResponse(&msg);
            }
        }
    }
    return NULL;
}