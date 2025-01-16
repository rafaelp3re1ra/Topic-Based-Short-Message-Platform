#include "managerUtils/managerUtils.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER;

int main()
{
    printf("Manager iniciado. Digite 'help' para ver os comandos.\n");

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&users_mutex, NULL);

    if (createPipe("fifoGlobal", O_RDWR | O_NONBLOCK) < 0)
    {
        printf("Erro ao criar pipe global\n");
        return EXIT_FAILURE;
    }

    pthread_t feedListener, expirationThread;
    pthread_create(&expirationThread, NULL, messageExpiration, NULL);
    pthread_create(&feedListener, NULL, listenToFeeds, (void *)"fifoGlobal");

    char buffer[256];
    while (managerRunning && readInput(buffer, sizeof(buffer)) > 0)
    {
        pthread_mutex_lock(&users_mutex);
        handleCommand(buffer);
        pthread_mutex_unlock(&users_mutex);
    }

    pthread_join(feedListener, NULL);
    pthread_join(expirationThread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&users_mutex);

    printf("Manager encerrado.\n");
    return 0;
}
