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
    int id;
    char message[MSG_SIZE];
    int messlen;
    int flags;
    struct sockaddr_in dest_addr;
    socklen_t addrlen;
    struct timeval tv;
    time_t tim;
} unackMsg;

//handle functions
int HandleReceive(int sockfd, char *buffer, struct sockaddr_in src_addr, int msglen);
int HandleACKMsgReceive(int id, char *buffer);
int HandleAppMsgReceive(int id, int sockfd, char *buf, struct sockaddr_in source_addr, socklen_t addr_len);
int HandleRetransmit();
void HandleReTransmit(int sockfd);
//other functions
unackMsg *find_empty_place_unAckTable();
void *runnerX(void *param);
int dropMessage(float p);

int sockfd_udp = -1;
int counter = 0;
struct sockaddr_in recv_source_addr;
socklen_t recv_addr_len = 0;
int recv_flags = 0;
int start_r_buffer = 0, end_r_buffer = 0, buffer_count = 0;
int cnt_trans = 0;
int unack_msg_last = -1;
int num_tranmissions = 0;

// Initialize table
unackMsg *unackTable;
recvMsg *recvMsgTable;
recvMsgID *recvMsgIDTable;

// Initailize threads
pthread_t tid;
pthread_mutex_t mutex;
pthread_mutexattr_t mutex_attribute;
//////arnabs/////////////////////////
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

    start_r_buffer = 0;
    end_r_buffer = 0;
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
//////////////////arnabs//////////////
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

    //for getting byte size
    if (len == -1)
        len = strlen(local_unack_msg->message);
    for (size_t i = len; i < len + sizeof(local_unack_msg->id); i++)
        local_unack_msg->message[i] = '\0';
    strcat(local_unack_msg->message + len + 1, (char *)&local_unack_msg->id);
    size_t no_of_bytes = len + sizeof(local_unack_msg->id);

    // assert no of bytes ==more than the required length
    assert(no_of_bytes == len + sizeof(local_unack_msg->id));
    local_unack_msg->messlen = no_of_bytes;
    local_unack_msg->flags = flags;
    local_unack_msg->dest_addr = *(struct sockaddr_in *)dest_addr;
    local_unack_msg->addrlen = addrlen;

    //udp sendto

    ssize_t Size = sendto(sockfd, local_unack_msg->message, local_unack_msg->messlen, local_unack_msg->flags, (struct sockaddr *)&local_unack_msg->dest_addr, local_unack_msg->addrlen);
    cnt_trans++;
    unack_msg_last++;
    return Size;
}
///////////////////arnabs////////////////
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
            strcpy(buf, recvMsgTable[start_r_buffer].message);
            recvMsgTable[start_r_buffer].dest_addr = recv_source_addr;
            start_r_buffer = (start_r_buffer + 1) % BUFFER_SIZE;
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
/////arnabs//////////////////////
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
///ours
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

/////arnabs and saisakeths////////////////////////////
void *runnerX(void *param)
{
    ///////////////////arnabs/////////////////////////
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
        else if (r)
        {
            if (FD_ISSET(sockfd_udp, &readfd))
            {
                ///////////////////////////////////////saisakeths//////////////////////////
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
                    int hr = HandleReceive(sockfd, buffer, src_addr, msg_len);
                }
                else
                {
                    // When Message is droped
                    printf("message dropped...\n");
                }
            }
        }
        else
        {
            timeout.tv_sec = TIMEOUT;
            timeout.tv_usec = TIMEOUT_USEC;
            // HandleRetransmit();
            HandleReTransmit(sockfd);
        }
    }
}
// saisakeths and arnabs
int HandleReceive(int sockfd, char *buffer, struct sockaddr_in src_addr, int msglen)
{
    int id;
    int *ret;
    msglen = strlen(buffer);
    ret = (int *)(buffer + msglen + 1);
    id = *ret;
    //arnabs///////

    if (!strcmp(buffer, "Acknow"))
    {
        return HandleACKMsgReceive(id, buffer);
    }
    else
    {
        struct sockaddr_in exa;
        return HandleAppMsgReceive(id, sockfd, buffer, src_addr, sizeof(exa));
    }
}
int check_dupli_recvidtable(int id)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (recvMsgIDTable[i].id == id)
        {
            return 1;
        }
    }
    return 0;
}
//arnabs /////////////////////////////////////////////

int HandleAppMsgReceive(int id, int sockfd, char *buf, struct sockaddr_in source_addr, socklen_t addr_len)
{
    int dupli_present;
    if ((dupli_present = check_dupli_recvidtable(id)) == 0)
    {
        strcpy(recvMsgTable[end_r_buffer].message, buf);
        recv_source_addr = source_addr;

        recv_addr_len = addr_len;
        buffer_count++;
        end_r_buffer = (end_r_buffer + 1) % BUFFER_SIZE;
        int j, i;
        for (i = 0; i < TABLE_SIZE; i++)
        {
            if (recvMsgIDTable[i].id == -1)
            {
                j = i;
                break;
            }
        }
        if (i == TABLE_SIZE)
            j = -1;
        if (i < 0)
            return -1;
        recvMsgIDTable[i].id = id;
        recvMsgIDTable[i].source_addr = source_addr;
        recvMsgIDTable[i].addr_len = addr_len;
    }
    //send ACK
    char ACK[BUFFER_SIZE];
    memset(ACK, '\0', sizeof(ACK));
    strcpy(ACK, "Acknow");
    //combine INt string
    int len = -1;
    if (len == -1)
        len = strlen(ACK);
    for (size_t i = len; i < len + sizeof(id); i++)
        buf[i] = '\0';
    strcat(ACK + len + 1, (char *)&id);
    size_t ret = len + sizeof(id);
    size_t r = sendto(sockfd_udp, ACK, ret, 0, (struct sockaddr *)&source_addr, addr_len);

    return 0;
}
//arnabs /////////////////////////////////////////////
int HandleACKMsgReceive(int id, char *buffer)
{
    printf("ACK %d\n", id);
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if (unackTable[i].id == id)
        {
            unackTable[i].id = -1;
            // free(unAckMsgTable[i].msg);
            unack_msg_last--;
            return 0;
        }
    }
    return -1;
}
//arnabs////////////////////////////////////////////
int HandleRetransmit()
{
    time_t time_now = time(NULL);
    for (int i = 0; i < TABLE_SIZE; i++)
    {

        if (unackTable[i].id != -1 &&
            unackTable[i].tv.tv_sec + TIMEOUT <= time_now)
        {
            ssize_t r = sendto(sockfd_udp, unackTable[i].message,
                               unackTable[i].messlen,
                               unackTable[i].flags,
                               (struct sockaddr *)&unackTable[i].dest_addr,
                               unackTable[i].addrlen);
            unackTable[i].tv.tv_sec = time_now;
            printf("Retransmiting : %d\n", unackTable[i].id);
            cnt_trans++;
            if (r < 0)
                return -1;
        }
    }
    return 0;
}
////saisakeths/////////////////////////////
void HandleReTransmit(int sockfd)
{
    int i;
    struct timeval curr_time;
    gettimeofday(&curr_time, NULL);
    //----------------------lock
    //  pthread_mutex_lock(&mutex);
    // Loop over the unacknowledged messages.
    for (i = 0; i <= unack_msg_last; i++)
    {
        // If the message has timed out,
        if (curr_time.tv_sec - unackTable[i].tv.tv_sec >= TIMEOUT && unackTable[i].id != -1)
        {
            // Retransmit the message
            printf("Retransmitting %d\n", unackTable[i].id);
            if (sendto(sockfd, unackTable[i].message, unackTable[i].messlen, 0, (const struct sockaddr *)&unackTable[i].dest_addr, sizeof(unackTable[i].dest_addr)) < 0)
            {
                perror("Unable to send");
                exit(1);
            }
            // Update the time for the message
            unackTable[i].tv.tv_sec = curr_time.tv_sec;
            unackTable[i].tv.tv_usec = curr_time.tv_usec;
            num_tranmissions++;
        }
    }
    // pthread_mutex_unlock(&mutex);
    //----------------------unlock
}
///both have written same
int dropMessage(float p)
{
    float rand_num = (float)rand() / ((float)RAND_MAX + 1);
    return rand_num < p;
}
