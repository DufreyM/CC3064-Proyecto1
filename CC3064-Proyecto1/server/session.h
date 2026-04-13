#ifndef SESSION_H
#define SESSION_H

/*
 * session.h
 * ---------
 * Interfaz de los threads que manejan la logica del protocolo por cliente.
 *
 * Cada vez que el servidor acepta una nueva conexion, crea un thread
 * que corre client_session_thread() para ese cliente.
 * Hay un unico thread global corriendo inactivity_watchdog_thread().
 */

/*
 * Contexto pasado al thread de sesion al crearlo.
 * Se asigna en heap (malloc) en server.c y el thread lo libera (free)
 * al inicio de su ejecucion.
 */
typedef struct {
    int sockfd;   /* File descriptor del socket TCP del cliente. */
    char ip[64];  /* IP del cliente en formato texto (ej. "192.168.1.5"). */
} SessionArgs;

/* Atiende todos los comandos de un cliente hasta que se desconecte o haga logout. */
void *client_session_thread(void *arg);

/* Monitorea periodicamente la inactividad y marca usuarios como INACTIVE. */
void *inactivity_watchdog_thread(void *arg);

#endif
