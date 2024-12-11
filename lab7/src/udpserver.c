#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <port> <buffer size>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int bufsize = atoi(argv[2]);
    int sockfd, n;
    char mesg[bufsize], ipadr[16];
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket problem");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (SADDR *)&servaddr, sizeof(struct sockaddr_in)) < 0) {
        perror("bind problem");
        exit(1);
    }
    printf("SERVER starts...\n");

    while (1) {
        unsigned int len = sizeof(struct sockaddr_in);

        if ((n = recvfrom(sockfd, mesg, bufsize, 0, (SADDR *)&cliaddr, &len)) < 0) {
            perror("recvfrom");
            exit(1);
        }
        mesg[n] = 0;

        printf("REQUEST %s      FROM %s : %d\n", mesg,
               inet_ntop(AF_INET, (void *)&cliaddr.sin_addr.s_addr, ipadr, 16),
               ntohs(cliaddr.sin_port));

        if (sendto(sockfd, mesg, n, 0, (SADDR *)&cliaddr, len) < 0) {
            perror("sendto");
            exit(1);
        }
    }
}
