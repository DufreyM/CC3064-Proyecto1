#include "client_manager.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static Client g_clients[MAX_CLIENTS];
static int g_client_count = 0;
static pthread_mutex_t g_clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void cm_init(void) {
    pthread_mutex_lock(&g_clients_mutex);
    memset(g_clients, 0, sizeof(g_clients));
    g_client_count = 0;
    pthread_mutex_unlock(&g_clients_mutex);
    printf("[INIT] Client manager inicializado\n");
}

static int is_valid_status(const char *status) {
    if (strcmp(status, STATUS_ACTIVO) == 0) return 1;
    if (strcmp(status, STATUS_OCUPADO) == 0) return 1;
    if (strcmp(status, STATUS_INACTIVO) == 0) return 1;
    return 0;
}

/* ================= ADD CLIENT ================= */
int cm_add_client(const char *username, const char *ip, int sockfd) {
    pthread_mutex_lock(&g_clients_mutex);

    if (g_client_count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&g_clients_mutex);
        printf("[ERROR] Servidor lleno\n");
        return -3;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!g_clients[i].active) continue;

        if (strcmp(g_clients[i].username, username) == 0) {
            pthread_mutex_unlock(&g_clients_mutex);
            printf("[ERROR] Username duplicado: %s\n", username);
            return -1;
        }

        if (strcmp(g_clients[i].ip, ip) == 0) {
            pthread_mutex_unlock(&g_clients_mutex);
            printf("[ERROR] IP duplicada: %s\n", ip);
            return -2;
        }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].active) continue;

        memset(&g_clients[i], 0, sizeof(Client));

        strncpy(g_clients[i].username, username, sizeof(g_clients[i].username) - 1);
        strncpy(g_clients[i].ip, ip, sizeof(g_clients[i].ip) - 1);
        strncpy(g_clients[i].status, STATUS_ACTIVO, sizeof(g_clients[i].status) - 1);

        g_clients[i].sockfd = sockfd;
        g_clients[i].active = 1;
        g_clients[i].last_activity = time(NULL);

        g_client_count++;

        printf("[ADD] Usuario %s agregado (%s)\n", username, ip);

        pthread_mutex_unlock(&g_clients_mutex);
        return 0;
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return -3;
}

/* ================= REMOVE ================= */
int cm_remove_client(const char *username) {
    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].active && strcmp(g_clients[i].username, username) == 0) {

            printf("[REMOVE] Usuario eliminado: %s\n", username);

            memset(&g_clients[i], 0, sizeof(Client));
            g_client_count--;

            pthread_mutex_unlock(&g_clients_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return -1;
}

int cm_remove_by_sockfd(int sockfd, char *out_username, int out_len) {
    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].active && g_clients[i].sockfd == sockfd) {

            if (out_username && out_len > 0) {
                strncpy(out_username, g_clients[i].username, out_len - 1);
                out_username[out_len - 1] = '\0';
            }

            printf("[REMOVE] Socket %d eliminado (%s)\n", sockfd, g_clients[i].username);

            memset(&g_clients[i], 0, sizeof(Client));
            g_client_count--;

            pthread_mutex_unlock(&g_clients_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return -1;
}

/* ================= FIND ================= */
int cm_find_client(const char *username, Client *out) {
    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].active && strcmp(g_clients[i].username, username) == 0) {
            if (out) *out = g_clients[i];
            pthread_mutex_unlock(&g_clients_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return -1;
}

/* ================= STATUS ================= */
int cm_set_status(const char *username, const char *status) {
    if (!is_valid_status(status)) return -2;

    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].active && strcmp(g_clients[i].username, username) == 0) {

            strncpy(g_clients[i].status, status, sizeof(g_clients[i].status) - 1);
            g_clients[i].last_activity = time(NULL);

            printf("[STATUS] %s -> %s\n", username, status);

            pthread_mutex_unlock(&g_clients_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return -1;
}

/* ================= ACTIVITY ================= */
int cm_update_activity(const char *username) {
    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].active && strcmp(g_clients[i].username, username) == 0) {
            g_clients[i].last_activity = time(NULL);
            pthread_mutex_unlock(&g_clients_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return -1;
}

int cm_reactivate_if_inactive(const char *username) {
    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].active && strcmp(g_clients[i].username, username) == 0) {

            g_clients[i].last_activity = time(NULL);

            if (strcmp(g_clients[i].status, STATUS_INACTIVO) == 0) {
                strncpy(g_clients[i].status, STATUS_ACTIVO, sizeof(g_clients[i].status) - 1);
                printf("[REACTIVATE] %s ahora ACTIVO\n", username);
                pthread_mutex_unlock(&g_clients_mutex);
                return 1;
            }

            pthread_mutex_unlock(&g_clients_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return -1;
}

/* ================= SOCKETS ================= */
int cm_get_sockets_except(int exclude_sockfd, int *out_sockets, int max_sockets) {
    int count = 0;

    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS && count < max_sockets; i++) {
        if (g_clients[i].active && g_clients[i].sockfd != exclude_sockfd) {
            out_sockets[count++] = g_clients[i].sockfd;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return count;
}

int cm_get_socket_by_username(const char *username) {
    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].active && strcmp(g_clients[i].username, username) == 0) {
            int sock = g_clients[i].sockfd;
            pthread_mutex_unlock(&g_clients_mutex);
            return sock;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return -1;
}

/* ================= LIST ================= */
int cm_build_user_list(char *buffer, int buffer_len) {
    int written = 0;
    buffer[0] = '\0';

    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!g_clients[i].active) continue;

        int needed = snprintf(buffer + written, buffer_len - written,
                              "%s,%s;", g_clients[i].username, g_clients[i].status);

        if (needed < 0 || written + needed >= buffer_len) {
            pthread_mutex_unlock(&g_clients_mutex);
            return -1;
        }

        written += needed;
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return 0;
}

/* ================= INFO ================= */
int cm_build_user_info(const char *username, char *buffer, int buffer_len) {
    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (g_clients[i].active && strcmp(g_clients[i].username, username) == 0) {
            snprintf(buffer, buffer_len, "%s,%s",
                     g_clients[i].ip, g_clients[i].status);

            pthread_mutex_unlock(&g_clients_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return -1;
}

/* ================= INACTIVITY ================= */
int cm_mark_inactive_clients(int timeout_sec, InactiveEvent *events, int max_events) {
    time_t now = time(NULL);
    int count = 0;

    pthread_mutex_lock(&g_clients_mutex);

    for (int i = 0; i < MAX_CLIENTS && count < max_events; i++) {
        if (!g_clients[i].active) continue;

        if (strcmp(g_clients[i].status, STATUS_INACTIVO) == 0) continue;

        if ((int)(now - g_clients[i].last_activity) > timeout_sec) {

            strncpy(g_clients[i].status, STATUS_INACTIVO, sizeof(g_clients[i].status) - 1);

            events[count].sockfd = g_clients[i].sockfd;
            size_t len = strlen(g_clients[i].username);

            if (len >= sizeof(events[count].username)) {
                len = sizeof(events[count].username) - 1;
            }

            memcpy(events[count].username, g_clients[i].username, len);
            events[count].username[len] = '\0';

            printf("[INACTIVE DETECTED] %s\n", g_clients[i].username);

            count++;
        }
    }

    pthread_mutex_unlock(&g_clients_mutex);
    return count;
}