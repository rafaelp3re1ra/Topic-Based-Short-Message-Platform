#ifndef MANAGERUTILS_H
#define MANAGERUTILS_H

#include "../utils.h"
#include "userManagement.h"

extern int managerRunning;

void handleCommand(const char *command);
void *listenToFeeds(void *arg);
void listUsers(void);
void shutdownManager();

#endif
