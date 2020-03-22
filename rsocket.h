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

// some Defines
#define SOCK_MRP 1

//recieve buffer struct
typedef struct _recievebuff
{
    char message[101];
    struct sockaddr_in source_addr;
    int messlen;
} recvbuffer;
// unackmess struct
typedef struct _unackbuff
{
    char message[101];
    int id;
    int messlen;
    struct timeval tv;
} unackmessbuffer;

//function prototypes
int r_socket(int domain, int type, int protocol);
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

#endif
