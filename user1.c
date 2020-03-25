#include "rsocket.h"

#define PORT1 52056
#define PORT2 52057

int main()
{
    char input[101];
    printf("Enter String\n");
    scanf("%s", input);
    int sockfd;
    sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    struct sockaddr_in user1_addr, user2_addr;
    if (sockfd < 0)
    {
        perror("socket creation error: user1\n");
        exit(EXIT_FAILURE);
    }
    memset(&user1_addr, 0, sizeof(user1_addr));
    //source address assigning
    user1_addr.sin_family = AF_INET;
    user1_addr.sin_addr.s_addr = INADDR_ANY;
    user1_addr.sin_port = htons(PORT1);
    //destination address assigning
    user2_addr.sin_family = AF_INET;
    user2_addr.sin_addr.s_addr = INADDR_ANY;
    user2_addr.sin_port = htons(PORT2);
    //bind to rsocket
    if (r_bind(sockfd, (const struct sockaddr *)&user1_addr, sizeof(user1_addr)) < 0)
    {
        perror("Bind error in user1\n");
        exit(1);
    }
    printf("Sender running....ie., user1\n");
    int i, len = strlen(input);
    // Send each character of the entered string one by one
    for (i = 0; i < len; i++)
    {
        int ret = r_sendto(sockfd, &input[i], 1, 0, (struct sockaddr *)&user2_addr, sizeof(user2_addr));
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
