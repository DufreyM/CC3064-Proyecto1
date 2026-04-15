/**
 * ============================================================
 * UTILIDADES DE RED
 * Archivo: net_utils.c
 *
 * Funciones auxiliares para asegurar el envío y recepción
 * completa de datos en sockets TCP.
 *
 * Nota: send() y recv() pueden procesar menos bytes de los
 * solicitados, por lo que se repite la operación hasta completar.
 * ============================================================
 */

#include "net_utils.h"
#include <sys/socket.h>

/* Envía todos los bytes del buffer */
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

/* Recibe exactamente len bytes */
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