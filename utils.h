#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

// Definições globais
#define MAX_TOPICS 20
#define MAX_MSG_LEN 300
#define MAX_TOPIC_NAME 20
#define MAX_USERNAME 20

// Tipos de mensagens
#define MSG_NORMAL 0      // Mensagem normal para um tópico
#define MSG_LIST 1        // Comando para listar tópicos
#define MSG_SHOW 2        // Comando para mostrar mensagens de um tópico
#define MSG_SUBSCRIBE 3   // Comando para subscrever um tópico
#define MSG_UNSUBSCRIBE 4 // Comando para cancelar subscrição
#define MSG_ERROR 5       // Mensagem de erro
#define MSG_SUCCESS 6     // Mensagem de sucesso

// Response types
#define RESP_NORMAL 0
#define RESP_LIST 1
#define RESP_ERROR 2
#define RESP_SUCCESS 3
#define RESP_TOPIC_CREATED 4
#define RESP_BYE_BYE 5

typedef struct
{
    char topic[MAX_TOPIC_NAME];
    char sender[MAX_USERNAME];
    char content[MAX_MSG_LEN];
    int duration;
    int pers; // Persistente(1) ou não(0)
    int type; // Tipo de mensagem (connect/disconnect/message/etc)
} Message;

int readInput(char *buffer, size_t size);
int sendMessage(int fd, const Message *msg);

#endif