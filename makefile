MANAGER_SRCS = manager.c utils.c managerUtils/managerUtils.c managerUtils/userManagement.c managerUtils/topicManagement.c
FEED_SRCS = feed.c utils.c feedUtils/feedUtils.c

all: manager feed

manager: $(MANAGER_SRCS)
	gcc -o manager $(MANAGER_SRCS) -pthread

feed: $(FEED_SRCS)
	gcc -o feed $(FEED_SRCS)

clean:
	rm -f manager feed *.o *.fifo *pipe* *fifo*