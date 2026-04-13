#ifndef NET_UTILS_H
#define NET_UTILS_H

int send_all(int sockfd, const void *buf, int len);
int recv_all(int sockfd, void *buf, int len);

#endif