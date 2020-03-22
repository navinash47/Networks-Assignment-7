#include "rsocket.h"

#define PORT1 53030
#define PORT2 53031

int main()
{
    char input[101];
    printf("Enter String\n");
    scanf("%s", input);
    int sockfd;
    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    struct sockaddr_in srcaddr, destaddr;
    if (sockfd < 0)
    {
        perror("socket creation error: user1\n");
        exit(EXIT_FAILURE);
    }
    memset(&srcaddr, 0, sizeof(srcaddr));
    //source address assigning
    srcaddr.sin_family = AF_INET;
    srcaddr.sin_addr.s_addr = INADDR_ANY;
    srcaddr.sin_port = htons(PORT1);
    //destination address assigning
    destaddr.sin_family = AF_INET;
    destaddr.sin_addr.s_addr = INADDR_ANY;
    destaddr.sin_port = htons(PORT2);
    //bind to rsocket
    if (r_bind(sockfd, (const struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0)
    {
        perror("Bind error in user1\n");
        exit(1);
    }
    printf("Sender running....\n");
    int i, len = strlen(input);
    // Send each character of the entered string one by one
    for (i = 0; i < len; i++)
    {
        int ret = r_sendto(sockfd, &input[i], 1, 0, (struct sockaddr *)&destaddr, sizeof(destaddr));
        if (ret < 0)
        {
            perror("Send error");
            exit(1);
        }
    }
    // Close the socket after sending
    r_close(sockfd);
    return 0;
}
