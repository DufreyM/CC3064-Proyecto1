/**
 * ============================================================
 * UTILIDADES DE RED - INTERFAZ
 * Archivo: net_utils.h
 *
 * Declara funciones para envío y recepción completa de datos
 * sobre sockets TCP.
 * ============================================================
 */

#ifndef NET_UTILS_H
#define NET_UTILS_H

/* Envía todos los bytes del buffer */
int send_all(int sockfd, const void *buf, int len);

/* Recibe exactamente len bytes */
int recv_all(int sockfd, void *buf, int len);

#endif