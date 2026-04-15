/**
 * ============================================================
 * SERVIDOR TCP - SISTEMA DE MENSAJERÍA
 * Archivo: servidor.c
 *
 * Inicializa el servidor, acepta conexiones entrantes y crea
 * un hilo por cada cliente. Incluye manejo de señales y un
 * watchdog para detectar inactividad.
 * ============================================================
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client_manager.h"
#include "session.h"

static int server_fd = -1; // socket del servidor

/* Maneja Ctrl+C para cerrar el servidor correctamente */
void handle_sigint(int sig) {
    (void)sig;
    printf("\n[INFO] Cerrando servidor...\n");
    if (server_fd != -1) {
        close(server_fd);
    }
    exit(0);
}

/* Crea, configura y deja listo el socket del servidor */
static int create_server_socket(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    /* Permite aceptar conexiones en cualquier interfaz */
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 16) < 0) {
        perror("listen");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: ./servidor <puerto>\n");
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Puerto invalido\n");
        return 1;
    }

    /* Evita cierre por SIGPIPE al escribir en sockets cerrados */
    signal(SIGPIPE, SIG_IGN);

    /* Manejo de Ctrl+C */
    signal(SIGINT, handle_sigint);

    /* Inicializar gestor de clientes */
    cm_init();

    server_fd = create_server_socket(port);
    if (server_fd < 0) {
        return 1;
    }

    /* Hilo watchdog para detectar inactividad */
    pthread_t watchdog_tid;
    if (pthread_create(&watchdog_tid, NULL, inactivity_watchdog_thread, NULL) != 0) {
        perror("pthread_create watchdog");
        close(server_fd);
        return 1;
    }
    pthread_detach(watchdog_tid);

    printf("=====================================\n");
    printf("Servidor iniciado correctamente\n");
    printf("Puerto: %d\n", port);
    printf("Esperando conexiones...\n");
    printf("=====================================\n");

    /* Loop principal: aceptar conexiones */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        /* Mostrar IP del cliente */
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("[NUEVA CONEXION] %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        SessionArgs *args = (SessionArgs *)malloc(sizeof(SessionArgs));
        if (args == NULL) {
            perror("malloc");
            close(client_fd);
            continue;
        }

        args->sockfd = client_fd;

        if (inet_ntop(AF_INET, &client_addr.sin_addr, args->ip, sizeof(args->ip)) == NULL) {
            strncpy(args->ip, "unknown", sizeof(args->ip) - 1);
            args->ip[sizeof(args->ip) - 1] = '\0';
        }

        /* Crear hilo por cliente */
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_session_thread, args) != 0) {
            perror("pthread_create session");
            close(client_fd);
            free(args);
            continue;
        }

        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}