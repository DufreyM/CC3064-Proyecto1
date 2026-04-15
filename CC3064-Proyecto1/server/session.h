/**
 * ============================================================
 * SESIONES DE CLIENTE - INTERFAZ
 * Archivo: session.h
 *
 * Declara los hilos encargados de manejar la comunicación
 * con clientes y la detección de inactividad en el servidor.
 * ============================================================
 */

#ifndef SESSION_H
#define SESSION_H

/* Argumentos para el hilo de sesión */
typedef struct {
    int sockfd;   /* socket del cliente */
    char ip[64];  /* IP en formato texto */
} SessionArgs;

/* Hilo que maneja la comunicación con un cliente */
void *client_session_thread(void *arg);

/* Hilo que monitorea inactividad de usuarios */
void *inactivity_watchdog_thread(void *arg);

#endif