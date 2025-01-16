#ifndef FEEDUTILS_H
#define FEEDUTILS_H

#include "../utils.h"

typedef struct
{
    char username[MAX_USERNAME];
    char pipe_name[50];
    int managerPipe;
    int responsePipe;
    int running;
} FeedContext;

void initFeed(FeedContext *ctx, const char *user);
void handle_input(FeedContext *ctx);
void shutdownFeed(FeedContext *ctx, int bye);
void *listen_manager(void *arg);
void handleCommandResponse(Message *msg);

#endif