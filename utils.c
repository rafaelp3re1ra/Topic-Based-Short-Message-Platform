#include "utils.h"

int readInput(char *buffer, size_t size)
{
    if (fgets(buffer, size, stdin) != NULL)
    {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }
        return len;
    }
    return -1;
}

int sendMessage(int fd, const Message *msg)
{
    return write(fd, msg, sizeof(Message));
}
