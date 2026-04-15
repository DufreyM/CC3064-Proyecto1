/**
 * ============================================================
 * CLIENTE TCP - SISTEMA DE MENSAJERÍA
 * Archivo: cliente.c
 *
 * Implementa un cliente que se conecta a un servidor TCP,
 * registra un usuario y permite enviar comandos al sistema.
 * Utiliza un hilo independiente para recibir mensajes.
 * ============================================================
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "command_parser.h"
#include "receiver.h"

#define BUFFER_SIZE 1024

/* Establece conexión TCP con el servidor */
static int connect_to_server(const char *ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);

    /* Convertir IP a formato binario */
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        fprintf(stderr, "[ERROR] IP invalida\n");
        close(sockfd);
        return -1;
    }

    printf("[INFO] Conectando a %s:%d...\n", ip, port);

    /* Intentar conexión */
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[ERROR] connect");
        close(sockfd);
        return -1;
    }

    printf("[INFO] Conexion establecida\n");
    return sockfd;
}

/* Muestra los comandos disponibles */
static void print_help(void) {
    printf("\nComandos disponibles:\n");
    printf("  /broadcast <mensaje>\n");
    printf("  /msg <usuario> <mensaje>\n");
    printf("  /status <ACTIVO|OCUPADO|INACTIVO>\n");
    printf("  /list\n");
    printf("  /info <usuario>\n");
    printf("  /help\n");
    printf("  /exit\n\n");
}

/* Envía texto al servidor */
static int send_text(int sockfd, const char *text) {
    return send(sockfd, text, strlen(text), 0);
}

/**
 * Función principal:
 * - Valida parámetros de entrada
 * - Se conecta al servidor
 * - Registra el usuario
 * - Inicia el hilo receptor
 * - Procesa comandos del usuario
 */
int main(int argc, char **argv) {
    int sockfd;
    int port;
    pthread_t rx_tid;
    char line[1200];

    /* Validación de argumentos */
    if (argc != 4) {
        fprintf(stderr, "Uso: ./cliente <username> <IP_servidor> <puerto>\n");
        return 1;
    }

    /* Validación de puerto */
    port = atoi(argv[3]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Puerto invalido\n");
        return 1;
    }

    /* Conexión al servidor */
    sockfd = connect_to_server(argv[2], port);
    if (sockfd < 0) return 1;

    /* Registro inicial del usuario */
    {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "REGISTER|%s\n", argv[1]);

        if (send_text(sockfd, buffer) <= 0) {
            printf("[ERROR] No se pudo registrar\n");
            close(sockfd);
            return 1;
        }
    }

    /* Creación del hilo receptor */
    if (pthread_create(&rx_tid, NULL, receiver_thread, &sockfd) != 0) {
        perror("pthread_create");
        close(sockfd);
        return 1;
    }

    print_help();
    printf("> ");

    /* Bucle principal de entrada de usuario */
    while (fgets(line, sizeof(line), stdin) != NULL) {

        ParsedCommand cmd;

        /* Interpretar comando */
        if (parse_input_line(line, &cmd) != 0) {
            printf("[ERROR] Comando invalido\n> ");
            continue;
        }

        char buffer[BUFFER_SIZE];

        /* Mostrar ayuda */
        if (cmd.type == PARSE_HELP) {
            print_help();
            printf("> ");
            continue;
        }

        /* Salir */
        if (cmd.type == PARSE_EXIT) {
            send_text(sockfd, "EXIT\n");
            break;
        }

        /* Mensaje global */
        if (cmd.type == PARSE_BROADCAST) {
            snprintf(buffer, sizeof(buffer), "BROADCAST|%s\n", cmd.arg2);
            if (send_text(sockfd, buffer) <= 0) break;
            printf("> ");
            continue;
        }

        /* Mensaje directo */
        if (cmd.type == PARSE_DIRECT) {
            snprintf(buffer, sizeof(buffer), "DIRECT|%s|%s\n", cmd.arg1, cmd.arg2);
            if (send_text(sockfd, buffer) <= 0) break;
            printf("> ");
            continue;
        }

        /* Cambio de estado */
        if (cmd.type == PARSE_STATUS) {
            snprintf(buffer, sizeof(buffer), "STATUS|%s\n", cmd.arg1);
            if (send_text(sockfd, buffer) <= 0) break;
            printf("> ");
            continue;
        }

        /* Solicitar lista de usuarios */
        if (cmd.type == PARSE_LIST) {
            send_text(sockfd, "LIST\n");
            printf("> ");
            continue;
        }

        /* Solicitar información de usuario */
        if (cmd.type == PARSE_INFO) {
            snprintf(buffer, sizeof(buffer), "INFO|%s\n", cmd.arg1);
            if (send_text(sockfd, buffer) <= 0) break;
            printf("> ");
            continue;
        }
    }

    /* Cierre de conexión */
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    /* Esperar hilo receptor */
    pthread_join(rx_tid, NULL);

    printf("[INFO] Cliente finalizado\n");
    return 0;
}