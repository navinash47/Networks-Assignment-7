#include "rsocket.h"

#define PORT1 52056
#define PORT2 52057

int main()
{
    int sockfd;
    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    struct sockaddr_in user1_addr, user2_addr;
    if (sockfd < 0)
    {
        perror("Error in socket creation");
        exit(1);
    }
    // Fill sender details user2.c
    memset(&user1_addr, 0, sizeof(user1_addr));
    user1_addr.sin_family = AF_INET;
    user1_addr.sin_addr.s_addr = INADDR_ANY;
    user1_addr.sin_port = htons(PORT1);
    // Fill receiver details user1.c
    user2_addr.sin_family = AF_INET;
    user2_addr.sin_addr.s_addr = INADDR_ANY;
    user2_addr.sin_port = htons(PORT2);
    // Bind the socket
    if (r_bind(sockfd, (const struct sockaddr *)&user2_addr, sizeof(user2_addr)) < 0)
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
        len = sizeof(user1_addr);
        r_recvfrom(sockfd, &input, 1, 0, (struct sockaddr *)&user1_addr, &len);
        printf("---------------------------\n");
        printf("Received %c\n", input);
        fflush(stdout);
        printf("---------------------------\n");
    }
    r_close(sockfd);
    return 0;
    return 0;
}
