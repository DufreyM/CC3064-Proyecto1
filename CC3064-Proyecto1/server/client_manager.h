/**
 * ============================================================
 * GESTOR DE CLIENTES - INTERFAZ
 * Archivo: client_manager.h
 *
 * Define estructuras y funciones para manejar clientes
 * conectados en el servidor de forma segura (thread-safe).
 * ============================================================
 */

#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

/* Estados posibles de un cliente */
#define STATUS_ACTIVO "ACTIVO"
#define STATUS_OCUPADO "OCUPADO"
#define STATUS_INACTIVO "INACTIVO"

#define MAX_CLIENTS 100

/* Representa un cliente conectado */
typedef struct {
    char username[32];
    char ip[INET_ADDRSTRLEN];
    char status[16];
    int sockfd;
    int active;
    time_t last_activity;
} Client;

/* Evento de usuario marcado como inactivo */
typedef struct {
    int sockfd;
    char username[32];
} InactiveEvent;

/* Inicializa el gestor */
void cm_init(void);

/* Agrega cliente (valida duplicados y capacidad) */
int cm_add_client(const char *username, const char *ip, int sockfd);

/* Elimina cliente por username */
int cm_remove_client(const char *username);

/* Elimina cliente por socket */
int cm_remove_by_sockfd(int sockfd, char *out_username, int out_len);

/* Busca cliente */
int cm_find_client(const char *username, Client *out);

/* Actualiza estado */
int cm_set_status(const char *username, const char *status);

/* Actualiza actividad */
int cm_update_activity(const char *username);

/* Reactiva usuario si estaba inactivo */
int cm_reactivate_if_inactive(const char *username);

/* Obtiene sockets activos (excepto uno) */
int cm_get_sockets_except(int exclude_sockfd, int *out_sockets, int max_sockets);

/* Obtiene socket por username */
int cm_get_socket_by_username(const char *username);

/* Construye lista de usuarios */
int cm_build_user_list(char *buffer, int buffer_len);

/* Construye información de un usuario */
int cm_build_user_info(const char *username, char *buffer, int buffer_len);

/* Marca clientes inactivos según timeout */
int cm_mark_inactive_clients(int timeout_sec, InactiveEvent *events, int max_events);

#endif