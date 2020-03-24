#include "rsocket.h"

//initialize mutex locks
pthread_t tid;
pthread_mutex_t mutex;
pthread_mutexattr_t mutex_attribute;
int r_socket(int domain, int type, int protocol)
{
    int sockfd = socket(domain, SOCK_DGRAM, protocol);
    if (sockfd < 0)
    {
        return -1;
    }
    else
    {
    }
}