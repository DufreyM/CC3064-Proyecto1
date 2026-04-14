#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H
#define STATUS_ACTIVO "ACTIVO"
#define STATUS_OCUPADO "OCUPADO"
#define STATUS_INACTIVO "INACTIVO"

#include <netinet/in.h>
#include <pthread.h>
#include <time.h>


#define MAX_CLIENTS 100

/*
 * client_manager.h
 * ----------------
 * API publica del modulo de gestion de clientes.
 *
 * Este modulo es el unico punto de acceso al estado global del servidor.
 * Centralizar todo el acceso aqui garantiza que el mutex siempre se use
 * correctamente y que ningun otro modulo manipule el arreglo directamente.
 *
 * Filosofia de diseno:
 *   - Las funciones adquieren y liberan el mutex internamente.
 *   - Los llamadores no deben preocuparse por concurrencia.
 *   - Las funciones devuelven datos por copia, nunca punteros internos.
 */

/*
 * Representa una sesion de cliente activa en memoria del servidor.
 *
 * Campos:
 *   username      : nombre de usuario unico en el sistema.
 *   ip            : direccion IP del cliente en formato texto ("1.2.3.4").
 *   status        : estado de presencia (ACTIVO / OCUPADO / INACTIVO).
 *   sockfd        : file descriptor del socket TCP de esta sesion.
 *   active        : 1 si el slot esta ocupado, 0 si esta libre.
 *   last_activity : timestamp unix de la ultima actividad del usuario.
 */
typedef struct {
    char username[32];
    char ip[INET_ADDRSTRLEN];
    char status[16];
    int sockfd;
    int active;
    time_t last_activity;
} Client;

/*
 * Evento producido por cm_mark_inactive_clients().
 * Contiene lo necesario para que el watchdog notifique al cliente
 * que su estado cambio a INACTIVE, sin acceder al arreglo global.
 */
typedef struct {
    int sockfd;
    char username[32];
} InactiveEvent;

/* Inicializa almacenamiento, contador y deja estado limpio al arrancar. */
void cm_init(void);
/*
 * Agrega un cliente validando duplicados por username e IP.
 * Retornos:
 *  0  => agregado
 * -1 => username duplicado
 * -2 => IP duplicada
 * -3 => servidor lleno
 */
int cm_add_client(const char *username, const char *ip, int sockfd);
/* Elimina por username. */
int cm_remove_client(const char *username);
/* Elimina por socket y opcionalmente retorna username removido. */
int cm_remove_by_sockfd(int sockfd, char *out_username, int out_len);
/* Busca cliente y devuelve copia de la estructura. */
int cm_find_client(const char *username, Client *out);
/*
 * Actualiza status de un usuario.
 * Retornos:
 *  0  => actualizado
 * -1 => usuario no encontrado
 * -2 => status invalido
 */
int cm_set_status(const char *username, const char *status);
/* Renueva marca de actividad del usuario. */
int cm_update_activity(const char *username);
/*
 * Reactiva usuario INACTIVE a ACTIVE y actualiza timestamp.
 * Retorna 1 si hubo cambio de status, 0 si ya estaba activo/ocupado, -1 si no existe.
 */
int cm_reactivate_if_inactive(const char *username);
/* Obtiene sockets activos, excluyendo uno (util para broadcast). */
int cm_get_sockets_except(int exclude_sockfd, int *out_sockets, int max_sockets);
/* Obtiene socket por username para mensajes directos. */
int cm_get_socket_by_username(const char *username);
/* Construye CSV de usuarios y status: user,status;... */
int cm_build_user_list(char *buffer, int buffer_len);
/* Construye info puntual: ip,status. */
int cm_build_user_info(const char *username, char *buffer, int buffer_len);
/* Marca usuarios inactivos segun timeout y produce lista de eventos. */
int cm_mark_inactive_clients(int timeout_sec, InactiveEvent *events, int max_events);

#endif
