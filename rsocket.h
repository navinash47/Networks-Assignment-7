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

// Custom defines
#define SOCK_MRP 1
#define MSG_SIZE 101

// Send messages struct
typedef struct _sendMsg
{
    int id;
    struct sockaddr_in source_addr;
    char message[MSG_SIZE];
} sendMsg;

// sendMsg* sendBuff;
// sendBuff = (sendMsg*)malloc(100*(sizeof(sendMsg)))

// Recieve messages struct
typedef struct _recvMsg
{
    char message[MSG_SIZE];
    struct sockaddr_in source_addr;
    int messlen;
} recvMsg;

// recvMsg* recvBuff;
// recvBuff = (recvMsg*)malloc(100*(sizeof(recvMsg)))

// Unacknowledged messages struct
typedef struct _unackMsg
{
    char message[MSG_SIZE];
    int id;
    int messlen;
    struct sockaddr_in dest_addr;
    struct timeval tv;
} unackMsg;

// Function prototypes
int r_socket(int domain, int type, int protocol);
int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

void *thread_X(void *param);

void HandleRetransmit(int sockfd);
void HandleReceive(int sockfd, char *buffer, const struct sockaddr *src_addr, int msg_len);
void HandleAppMsgRecv(int sockfd, char *buffer, const struct sockaddr *src_addr, int msg_len);
void sendAck(int id, int sockfd, const struct sockaddr *dest_addr);
void HandleACKMsgRecv(char *buffer);

int min(int a, int b);
int dropMessage(float p);
int r_close(int sfd);

#endif
