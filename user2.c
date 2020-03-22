#include "rsocket.h"

#define PORT1 53030
#define PORT2 53031

int main()
{
    int sockfd;
    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    struct sockaddr_in srcaddr, destaddr;
    if (sockfd < 0)
    {
        perror("Error in socket creation");
        exit(1);
    }
    // Fill sender details user2.c
    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.sin_family = AF_INET;
    srcaddr.sin_addr.s_addr = INADDR_ANY;
    srcaddr.sin_port = htons(PORT1);
    // Fill receiver details user1.c
    destaddr.sin_family = AF_INET;
    destaddr.sin_addr.s_addr = INADDR_ANY;
    destaddr.sin_port = htons(PORT2);
    // Bind the socket
    if (r_bind(sockfd, (const struct sockaddr *)&destaddr, sizeof(destaddr)) < 0)
    {
        perror("Bind error");
        exit(1);
    }
    printf("Receiver running\n");
    char input;
    int len;
    // Receive all characters in a loop.
    // Here, we are not terminating the loop as we don't know when the receiver will
    // close its end and since the order of delivery is not preserved, we can't use end markers like \0
    while (1)
    {
        len = sizeof(srcaddr);
        r_recvfrom(sockfd, &input, 1, 0, (struct sockaddr *)&srcaddr, &len);
        printf("---------------------------\n");
        printf("Received %c\n", input);
        fflush(stdout);
        printf("---------------------------\n");
    }
    r_close(sockfd);
    return 0;
    return 0;
}
