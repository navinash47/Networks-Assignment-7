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
void HandleRetransmit(int sockfd);
void HandleReceive(int sockfd, char *buffer, const struct sockaddr *src_addr, int msg_len);
void HandleAppMsgRecv(int sockfd, char *buffer, const struct sockaddr *src_addr, int msg_len);
void sendAck(int id, int sockfd, const struct sockaddr *dest_addr);
void HandleACKMsgRecv(char *buffer);
//other functions
unackMsg *find_empty_place_unAckTable();

int min(int a, int b);
int dropMessage(float p);

int sockfd_udp = -1;
int counter = 0;

// Initialize table
unackMsg *unackTable;
recvMsg *recvMsgTable;
recvMsgID *recvMsgIDTable;

// Initailize threads
pthread_t tid;
pthread_mutex_t mutex;
pthread_mutexattr_t mutex_attribute;
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
