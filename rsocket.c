#include "rsocket.h"

//intialiaze sockfd_udp
int sockfd_udp = -1;
//initialize mutex locks
pthread_t tid;
pthread_mutex_t mutex;
pthread_mutexattr_t mutex_attribute;
int r_socket(int domain, int type, int protocol)
{
    if (type != SOCK_MRP)
        return -1;
    sockfd_udp = socket(domain, SOCK_DGRAM, protocol);
    if (sockfd_udp < 0)
    {
        return -1;
    }
    else
    {
    }
}