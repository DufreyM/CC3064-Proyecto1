#include "receiver.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

/*
 * Thread receptor:
 * Lee texto plano del servidor y lo interpreta usando '|'
 */
void *receiver_thread(void *arg) {
    int sockfd = *(int *)arg;
    char buffer[BUFFER_SIZE];

    while (1) {

        int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

        if (n <= 0) {
            printf("\n[INFO] Conexion cerrada por el servidor\n> ");
            fflush(stdout);
            break;
        }

        buffer[n] = '\0';

        /* Puede venir más de un mensaje, procesamos línea por línea */
        char *line = strtok(buffer, "\n");

        while (line != NULL) {

            /* Salto de línea para no romper input */
            printf("\n");

            char temp[BUFFER_SIZE];
            strncpy(temp, line, sizeof(temp) - 1);
            temp[sizeof(temp) - 1] = '\0';

            char *cmd = strtok(temp, "|");
            char *arg1 = strtok(NULL, "|");
            char *arg2 = strtok(NULL, "|");
            char *arg3 = strtok(NULL, "|");

            if (!cmd) {
                line = strtok(NULL, "\n");
                continue;
            }

            /* ================= MSG ================= */
            if (strcmp(cmd, "MSG") == 0) {

                if (arg2 && strcmp(arg2, "ALL") == 0) {
                    printf("[BROADCAST] %s: %s\n", arg1 ? arg1 : "?", arg3 ? arg3 : "");
                } else {
                    printf("[PRIVADO] %s -> %s: %s\n",
                           arg1 ? arg1 : "?",
                           arg2 ? arg2 : "?",
                           arg3 ? arg3 : "");
                }
            }

            /* ================= OK ================= */
            else if (strcmp(cmd, "OK") == 0) {
                printf("[OK] %s\n", arg1 ? arg1 : "");
            }

            /* ================= ERROR ================= */
            else if (strcmp(cmd, "ERROR") == 0) {
                printf("[ERROR] %s\n", arg1 ? arg1 : "");
            }

            /* ================= LIST ================= */
            else if (strcmp(cmd, "LIST") == 0) {
                printf("[USUARIOS]\n%s\n", arg1 ? arg1 : "");
            }

            /* ================= INFO ================= */
            else if (strcmp(cmd, "INFO") == 0) {
                printf("[INFO]\n%s\n", arg1 ? arg1 : "");
            }

            /* ================= DESCONECTADO ================= */
            else if (strcmp(cmd, "DISCONNECTED") == 0) {
                printf("[AVISO] Usuario desconectado: %s\n", arg1 ? arg1 : "");
            }

            /* ================= OTROS ================= */
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