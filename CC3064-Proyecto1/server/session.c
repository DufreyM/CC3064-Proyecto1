/**
 * ============================================================
 * SESIONES DE CLIENTE - SERVIDOR
 * Archivo: session.c
 *
 * Maneja la comunicación con cada cliente en un hilo independiente.
 * Procesa comandos del protocolo, envía respuestas y gestiona
 * desconexión e inactividad.
 * ============================================================
 */

#include "session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_manager.h"

#define BUFFER_SIZE 2048
#define INACTIVITY_TIMEOUT 20

/* Envía respuesta al cliente */
static void send_response(int sockfd, const char *msg) {
    send(sockfd, msg, strlen(msg), 0);
}

/* Procesa comandos del protocolo */
static void handle_command(int sockfd, char *line, char *username, int *registered, const char *ip) {

    char *cmd = strtok(line, "|");
    char *arg1 = strtok(NULL, "|");
    char *arg2 = strtok(NULL, "|");

    if (!cmd) return;

    /* ================= REGISTER ================= */
    if (!(*registered)) {
        if (strcmp(cmd, "REGISTER") != 0) {
            send_response(sockfd, "ERROR|Primero debes registrarte\n");
            return;
        }

        if (!arg1) {
            send_response(sockfd, "ERROR|Username invalido\n");
            return;
        }

        int rc = cm_add_client(arg1, ip, sockfd);

        if (rc == -1) {
            send_response(sockfd, "ERROR|Usuario ya existe\n");
            return;
        }
        if (rc == -2) {
            send_response(sockfd, "ERROR|IP duplicada\n");
            return;
        }

        strcpy(username, arg1);
        *registered = 1;

        char msg[128];
        snprintf(msg, sizeof(msg), "OK|Bienvenido %s\n", username);
        send_response(sockfd, msg);

        printf("[REGISTER] %s\n", username);
        return;
    }

    /* ================= BROADCAST ================= */
    if (strcmp(cmd, "BROADCAST") == 0) {
        if (!arg1) return;

        int sockets[MAX_CLIENTS];
        int count = cm_get_sockets_except(sockfd, sockets, MAX_CLIENTS);

        char msg[1024];
        snprintf(msg, sizeof(msg), "MSG|%s|ALL|%s\n", username, arg1);

        for (int i = 0; i < count; i++) {
            send_response(sockets[i], msg);
        }

        send_response(sockfd, "OK|Broadcast enviado\n");
        return;
    }

    /* ================= DIRECT ================= */
    if (strcmp(cmd, "DIRECT") == 0) {
        if (!arg1 || !arg2) return;

        int target = cm_get_socket_by_username(arg1);
        if (target < 0) {
            send_response(sockfd, "ERROR|Usuario no encontrado\n");
            return;
        }

        char msg[1024];
        snprintf(msg, sizeof(msg), "MSG|%s|%s|%s\n", username, arg1, arg2);

        send_response(target, msg);
        send_response(sockfd, "OK|Mensaje enviado\n");
        return;
    }

    /* ================= LIST ================= */
    if (strcmp(cmd, "LIST") == 0) {
        char listbuf[1024];
        if (cm_build_user_list(listbuf, sizeof(listbuf)) != 0) {
            send_response(sockfd, "ERROR|No se pudo construir lista\n");
            return;
        }

        char msg[1100];
        snprintf(msg, sizeof(msg), "LIST|%s\n", listbuf);
        send_response(sockfd, msg);
        return;
    }

    /* ================= INFO ================= */
    if (strcmp(cmd, "INFO") == 0) {
        if (!arg1) return;

        char info[1024];
        if (cm_build_user_info(arg1, info, sizeof(info)) != 0) {
            send_response(sockfd, "ERROR|Usuario no existe\n");
            return;
        }

        char msg[1100];
        snprintf(msg, sizeof(msg), "INFO|%s\n", info);
        send_response(sockfd, msg);
        return;
    }

    /* ================= STATUS ================= */
    if (strcmp(cmd, "STATUS") == 0) {
        if (!arg1) return;

        if (cm_set_status(username, arg1) != 0) {
            send_response(sockfd, "ERROR|Status invalido\n");
            return;
        }

        send_response(sockfd, "OK|Status actualizado\n");
        return;
    }

    /* ================= EXIT ================= */
    if (strcmp(cmd, "EXIT") == 0) {
        send_response(sockfd, "OK|Bye\n");
        return;
    }

    send_response(sockfd, "ERROR|Comando invalido\n");
}

/* Hilo por cliente */
void *client_session_thread(void *arg) {
    SessionArgs *args = (SessionArgs *)arg;

    int sockfd = args->sockfd;
    char ip[64];
    strcpy(ip, args->ip);

    free(args);

    char buffer[BUFFER_SIZE];
    char username[32] = {0};
    int registered = 0;

    while (1) {
        int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;

        buffer[n] = '\0';

        /* Procesar múltiples mensajes */
        char *line = strtok(buffer, "\n");

        while (line != NULL) {

            line[strcspn(line, "\r")] = '\0';

            char temp[BUFFER_SIZE];
            strncpy(temp, line, sizeof(temp) - 1);
            temp[sizeof(temp) - 1] = '\0';

            handle_command(sockfd, temp, username, &registered, ip);

            line = strtok(NULL, "\n");
        }
    }

    /* Cleanup al desconectar */
    if (registered) {
        char removed[32] = {0};

        if (cm_remove_by_sockfd(sockfd, removed, sizeof(removed)) == 0) {
            printf("[DISCONNECT] %s\n", removed);

            int sockets[MAX_CLIENTS];
            int count = cm_get_sockets_except(sockfd, sockets, MAX_CLIENTS);

            char msg[128];
            snprintf(msg, sizeof(msg), "DISCONNECTED|%s\n", removed);

            for (int i = 0; i < count; i++) {
                send_response(sockets[i], msg);
            }
        }
    }

    close(sockfd);
    return NULL;
}

/* Hilo watchdog para inactividad */
void *inactivity_watchdog_thread(void *arg) {
    (void)arg;

    while (1) {
        InactiveEvent events[MAX_CLIENTS];
        int count = cm_mark_inactive_clients(INACTIVITY_TIMEOUT, events, MAX_CLIENTS);

        for (int i = 0; i < count; i++) {
            char msg[128];
            snprintf(msg, sizeof(msg), "MSG|SERVER|%s|Tu status cambio a INACTIVO\n",
                     events[i].username);

            send_response(events[i].sockfd, msg);
        }

        sleep(2);
    }

    return NULL;
}