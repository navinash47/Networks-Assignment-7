#ifndef RSOCKET_H
#define RSOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <signal.h>

// Custom defines
#define SOCK_MRP 1
#define TABLE_SIZE 100
#define MSG_SIZE 101
#define TIMEOUT 2
#define TIMEOUT_USEC 0
#define DROP_PROBALITY 0.2
#define BUFFER_SIZE 100

// Function prototypes
int r_socket(int domain, int type, int protocol);
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

void *thread_X(void *param);

int r_close(int sfd);

#endif
