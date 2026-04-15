/**
 * ============================================================
 * RECEPTOR DE MENSAJES - CLIENTE (HILO)
 * Archivo: receiver.c
 *
 * Hilo encargado de recibir mensajes del servidor y mostrarlos
 * en consola. Procesa respuestas usando el protocolo basado
 * en texto con delimitadores '|' y '\n'.
 * ============================================================
 */

#include "receiver.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

/**
 * Recibe datos del servidor de forma continua, separa los
 * mensajes por línea y los interpreta según el protocolo.
 * Permite recepción asíncrona mientras el cliente envía datos.
 */
void *receiver_thread(void *arg) {
    int sockfd = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {

        /* Leer datos del socket */
        int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

        /* Conexión cerrada o error */
        if (n <= 0) {
            printf("\n[INFO] Conexion cerrada por el servidor\n> ");
            fflush(stdout);
            break;
        }

        buffer[n] = '\0';

        /* Procesar múltiples mensajes en el buffer */
        char *line = strtok(buffer, "\n");

        while (line != NULL) {

            printf("\n");

            /* Copia para no modificar el buffer original */
            char temp[BUFFER_SIZE];
            strncpy(temp, line, sizeof(temp) - 1);
            temp[sizeof(temp) - 1] = '\0';

            /* Separar campos del protocolo */
            char *cmd = strtok(temp, "|");
            char *arg1 = strtok(NULL, "|");
            char *arg2 = strtok(NULL, "|");
            char *arg3 = strtok(NULL, "|");

            if (!cmd) {
                line = strtok(NULL, "\n");
                continue;
            }

            /* Mensajes */
            if (strcmp(cmd, "MSG") == 0) {

                if (arg2 && strcmp(arg2, "ALL") == 0) {
                    printf("[BROADCAST] %s: %s\n",
                           arg1 ? arg1 : "?",
                           arg3 ? arg3 : "");
                } else {
                    printf("[PRIVADO] %s -> %s: %s\n",
                           arg1 ? arg1 : "?",
                           arg2 ? arg2 : "?",
                           arg3 ? arg3 : "");
                }
            }

            /* Respuestas OK */
            else if (strcmp(cmd, "OK") == 0) {
                printf("[OK] %s\n", arg1 ? arg1 : "");
            }

            /* Errores */
            else if (strcmp(cmd, "ERROR") == 0) {
                printf("[ERROR] %s\n", arg1 ? arg1 : "");
            }

            /* Lista de usuarios */
            else if (strcmp(cmd, "LIST") == 0) {
                printf("[USUARIOS]\n%s\n", arg1 ? arg1 : "");
            }

            /* Información de usuario */
            else if (strcmp(cmd, "INFO") == 0) {
                printf("[INFO]\n%s\n", arg1 ? arg1 : "");
            }

            /* Usuario desconectado */
            else if (strcmp(cmd, "DISCONNECTED") == 0) {
                printf("[AVISO] Usuario desconectado: %s\n",
                       arg1 ? arg1 : "");
            }

            /* Mensaje no reconocido */
            else {
                printf("[DESCONOCIDO] %s\n", line);
            }

            line = strtok(NULL, "\n");
        }

        printf("> ");
        fflush(stdout);
    }

    return NULL;
}