#include "feedUtils/feedUtils.h"

pthread_mutex_t feed_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Uso: ./feed <userName>\n");
        return 1;
    }

    pthread_mutex_init(&feed_mutex, NULL);

    FeedContext ctx;
    initFeed(&ctx, argv[1]);

    pthread_t thread;
    pthread_create(&thread, NULL, listen_manager, &ctx);

    while (ctx.running)
    {
        pthread_mutex_lock(&feed_mutex);
        handle_input(&ctx);
        pthread_mutex_unlock(&feed_mutex);
    }

    pthread_join(thread, NULL);
    pthread_mutex_destroy(&feed_mutex);

    return 0;
}