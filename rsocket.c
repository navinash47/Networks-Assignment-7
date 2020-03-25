#include "rsocket.h"

// Recieve messages struct
typedef struct _recvMsg
{
    int id;
    char message[MSG_SIZE];
    struct sockaddr_in dest_addr;
} recvMsg;

// Received messages id
typedef struct _recvMsgID
{
    int id;
    struct sockaddr_in source_addr;
    socklen_t addr_len;
} recvMsgID;

// Unacknowledged messages struct
typedef struct _unackMsg
{
    char message[MSG_SIZE];
    int id;
    int messlen;
    int flags;
    struct sockaddr_in dest_addr;
    socklen_t addrlen;
    struct timeval tv;
} unackMsg;

// Handle functions
// int HandleACKMsgReceive(int id);
// int HandleAppMsgReceive(int id, char *buf, struct sockaddr_in source_addr, socklen_t addr_len);
int HandleACKMsgReceive(int id, char *buffer);
int HandleAppMsgReceive(int id, int sockfd, char *buf, struct sockaddr_in source_addr, socklen_t addr_len);
int getEmptyPlaceRecvid();
size_t combineIntString(int id, char *buf, int len);
void breakIntString(int *id, char *buf, int len);
int delFromUnackTable(int id);
int HandleReceive();
int HandleRetransmit();
//other functions
unackMsg *find_empty_place_unAckTable();
void *runnerX(void *param);

int min(int a, int b);
int dropMessage(float p);

int sockfd_udp = -1;
int counter = 0;
struct sockaddr_in recv_source_addr;
socklen_t recv_addr_len = 0;
int recv_flags = 0;
int start_rb = 0, end_rb = 0, buffer_count = 0;

// Initialize table
unackMsg *unackTable;
recvMsg *recvMsgTable;
recvMsgID *recvMsgIDTable;

// Initailize threads
pthread_t tid;
pthread_mutex_t mutex;
pthread_mutexattr_t mutex_attribute;

// Functions
int r_socket(int domain, int type, int protocol)
{
    if (type != SOCK_MRP)
        return -1;

    if ((sockfd_udp = socket(domain, SOCK_DGRAM, protocol)) < 0)
        return sockfd_udp;

    // Malloc all tables
    unackTable = (unackMsg *)malloc(TABLE_SIZE * sizeof(unackMsg));
    recvMsgTable = (recvMsg *)malloc(TABLE_SIZE * sizeof(recvMsg));
    recvMsgIDTable = (recvMsgID *)malloc(TABLE_SIZE * sizeof(recvMsgID));

    // Initialise all to unused
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        recvMsgIDTable[i].id = -1;
        unackTable[i].id = -1;
    }
    pthread_mutex_init(&mutex, NULL);
    start_rb = 0;
    end_rb = 0;
    buffer_count = 0;

    int *param = (int *)malloc(sizeof(int));
    *param = sockfd_udp;

    // Thread to handle this socket
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    int ret = pthread_create(&tid, &attr, runnerX, param);
    if (ret < 0)
        return -1;
    return sockfd_udp;
}

int r_bind(int socket, const struct sockaddr *address, socklen_t address_len)
{
    // General UDP bind
    return bind(socket, address, address_len);
}

ssize_t r_sendto(int sockfd, const void *buffer, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
    if (sockfd != sockfd_udp)
        return -1;
    int count = ++counter;
    char *buff = (char *)buffer;

    //check if unackTable has an empty place
    unackMsg *local_unack_msg = find_empty_place_unAckTable();
    if (local_unack_msg == NULL)
        return -1;
    local_unack_msg->id = count;
    strcpy(local_unack_msg->message, buff);

    //for getting bte size
    if (len == -1)
        len = strlen(local_unack_msg->message);
    for (size_t i = len; i < len + sizeof(local_unack_msg->id); i++)
        local_unack_msg->message[i] = '\0';
    strcat(local_unack_msg->message + len + 1, (char *)&local_unack_msg->id);
    size_t no_of_bytes = len + sizeof(local_unack_msg->id);

    // assert no of bytes
    assert(no_of_bytes == len + sizeof(local_unack_msg->id));
    local_unack_msg->messlen = no_of_bytes;
    local_unack_msg->flags = flags;
    local_unack_msg->dest_addr = *(struct sockaddr_in *)dest_addr;
    local_unack_msg->addrlen = addrlen;

    //udp sendto
    ssize_t Size = sendto(sockfd, local_unack_msg->message, local_unack_msg->messlen, local_unack_msg->flags, (struct sockaddr *)&local_unack_msg->dest_addr, local_unack_msg->addrlen);
    return Size;
}

ssize_t r_recvfrom(int sockfd, void *buffer, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
    char *buf = (char *)buffer;

    if (sockfd != sockfd_udp)
        return -1;
    while (1)
    {
        if (buffer_count > 0)
        {
            buffer_count--;
            strcpy(buf, recvMsgTable[start_rb].message);
            recvMsgTable[start_rb].dest_addr = recv_source_addr;
            start_rb = (start_rb + 1) % BUFFER_SIZE;
            if (len >= 0 && len < strlen(buf))
            {
                buf[len] = '\0';
            }
            len = strlen(buf);
            *src_addr = *(struct sockaddr *)&recv_source_addr;
            *addrlen = recv_addr_len;
            recv_flags = flags;
            return len;
        }
        else if (flags == MSG_DONTWAIT)
            break;
        else
            sleep(0.001);
    }
}

int r_close(int sockfd)
{
    if (sockfd != sockfd_udp)
        return -1;
    while (1)
    {
        int flag = 0;
        for (int i = 0; i < TABLE_SIZE; i++)
        {
            if (unackTable[i].id != -1)
                flag = 1;
        }
        if (flag == 0)
            break;
    }
    pthread_kill(tid, SIGKILL);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        free(unackTable[i].message);
    }
    free(unackTable);
    free(recvMsgIDTable);
    free(recvMsgTable);
    return close(sockfd);
}

ssize_t sendACK(int id, struct sockaddr_in addr, socklen_t addr_len)
{
}

unackMsg *find_empty_place_unAckTable()
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (unackTable[i].id == -1)
        {
            return &unackTable[i];
        }
    }
    return NULL;
}

void *runnerX(void *param)
{
    int sockfd = *((int *)param);
    fd_set readfd;
    int nfds = sockfd + 1;
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = TIMEOUT_USEC;

    int r;
    while (1)
    {
        FD_ZERO(&readfd);
        FD_SET(sockfd_udp, &readfd);

        r = select(sockfd_udp + 1, &readfd, NULL, NULL, &timeout);
        if (r < 0)
        {
            perror("Select Failed\n");
        }
        else if (r > 0)
        {
            if (FD_ISSET(sockfd_udp, &readfd))
            {
                //came out when received a message
                char buffer[BUFFER_SIZE];
                bzero(buffer, BUFFER_SIZE);
                struct sockaddr_in src_addr;
                int len;
                len = sizeof(src_addr);
                // Receive message
                int msg_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&src_addr, &len);
                // Compute probability for dropping
                if (dropMessage(DROP_PROBALITY) == 0)
                {
                    // If not dropping, handle the received message
                    HandleReceive(sockfd, buffer, (const struct sockaddr *)&src_addr, msg_len);
                }
                else
                {
                    // Else do nothing
                    printf("Dropping message\n");
                }
                // HandleReceive();
            }
        }
        else
        {
            timeout.tv_sec = TIMEOUT;
            timeout.tv_usec = TIMEOUT_USEC;
            HandleRetransmit();
        }
    }
}

int HandleACKMsgReceive(int id, char *buffer)
{
    printf("ACK %d\n", id);
    return delFromUnackTable(id);
}

int HandleAppMsgReceive(int id, int sockfd, char *buf, struct sockaddr_in source_addr, socklen_t addr_len)
{
    
}

int HandleReceive(int sockfd, char *buffer, const struct sockaddr *src_addr, int msg_len)
{
    // If the message is an application message
    int id;
    breakIntString(&id, buffer, msg_len);

    if (strcmp(buffer, "ACK"))
    {
        HandleAppMsgRecv(id, sockfd, buffer, (const struct sockaddr *)src_addr, msg_len);
    }
    // If the message is an acknowledgemet
    else
    {
        HandleACKMsgRecv(id, buffer);
    }
}

int HandleRetransmit()
{
}

int dropMessage(float p)
{
    float rand_num = (float)rand() / ((float)RAND_MAX + 1);
    return rand_num < p;
}

int delFromUnackTable(int id)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (unackTable[i].id == id)
        {
            unackTable[i].id = -1;
            // free(unAckMsgTable[i].msg);
            return 0;
        }
    }
    return -1;
}

void breakIntString(int *id, char *buf, int len)
{
    int *ret;
    len = strlen(buf);
    ret = (int *)(buf + len + 1);
    *id = *ret;
}