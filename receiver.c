#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "receiver.h"
#include "decoder.h"

int run_receiver(int port)
{
    int sockfd;

    struct sockaddr_in local;
    struct sockaddr_in sender;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset(&local, 0, sizeof(local));

    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(port);

    if (bind(
            sockfd,
            (struct sockaddr *)&local,
            sizeof(local)
        ) < 0) {

        perror("bind");
        close(sockfd);
        return 1;
    }

    if (!isatty(STDOUT_FILENO)) {
        printf("Listening on UDP %d\n", port);
    }

    Decoder decoder;
    decoder_init(&decoder);

    while (1) {

        char buffer[1024];

        socklen_t sender_len = sizeof(sender);

        ssize_t n = recvfrom(
            sockfd,
            buffer,
            sizeof(buffer) - 1,
            0,
            (struct sockaddr *)&sender,
            &sender_len
        );

        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';

        char ip[64];

        inet_ntop(
            AF_INET,
            &sender.sin_addr,
            ip,
            sizeof(ip)
        );

        for (ssize_t i = 0; i < n; i++) {
            if (buffer[i] == '0') {
                decoder_feed_bit(&decoder, 0);
            } else if (buffer[i] == '1') {
                decoder_feed_bit(&decoder, 1);
            }
        }
    }

    return 0;
}
