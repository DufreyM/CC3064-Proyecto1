#include "net_utils.h"
#include <sys/socket.h>

int send_all(int sockfd, const void *buf, int len) {
    int total = 0;
    const char *p = buf;

    while (total < len) {
        int n = send(sockfd, p + total, len - total, 0);
        if (n <= 0) return -1;
        total += n;
    }
    return 0;
}

int recv_all(int sockfd, void *buf, int len) {
    int total = 0;
    char *p = buf;

    while (total < len) {
        int n = recv(sockfd, p + total, len - total, 0);
        if (n <= 0) return -1;
        total += n;
    }
    return 0;
}