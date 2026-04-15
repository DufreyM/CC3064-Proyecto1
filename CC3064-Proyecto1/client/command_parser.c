/**
 * ============================================================
 * PARSER DE COMANDOS - CLIENTE
 * Archivo: command_parser.c
 *
 * Interpreta los comandos ingresados por el usuario y los
 * convierte en una estructura ParsedCommand para su uso interno.
 *
 * Soporta comandos como: broadcast, msg, status, list, info,
 * help y exit. Incluye validación y normalización básica.
 * ============================================================
 */

#include "command_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

/* ================= UTILIDADES ================= */

/* Avanza hasta el siguiente carácter no espacio */
static const char *skip_spaces(const char *s) {
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    return s;
}

/* Convierte una cadena a mayúsculas */
static void to_uppercase(char *s) {
    for (; *s; s++) {
        *s = (char)toupper((unsigned char)*s);
    }
}

/* Copia segura de strings */
static void copy_token(char *dst, int dst_len, const char *src) {
    snprintf(dst, (size_t)dst_len, "%s", src);
}

/* ============================================================
 * Analiza una línea de entrada y la convierte en un comando
 * estructurado (ParsedCommand).
 * Retorna 0 si es válido, -1 si hay error.
 * ============================================================ */
int parse_input_line(const char *line, ParsedCommand *out) {
    char cmd[64] = {0};
    const char *p;

    /* Inicialización */
    memset(out, 0, sizeof(ParsedCommand));
    out->type = PARSE_INVALID;

    if (line == NULL) return -1;

    /* Ignorar espacios iniciales */
    p = skip_spaces(line);
    if (*p == '\0') return -1;

    /* Leer comando */
    if (sscanf(p, "%63s", cmd) != 1) return -1;

    /* Comparación sin importar mayúsculas */
    to_uppercase(cmd);

    /* HELP */
    if (strcmp(cmd, "/HELP") == 0) {
        out->type = PARSE_HELP;
        return 0;
    }

    /* EXIT */
    if (strcmp(cmd, "/EXIT") == 0) {
        out->type = PARSE_EXIT;
        return 0;
    }

    /* LIST */
    if (strcmp(cmd, "/LIST") == 0) {
        out->type = PARSE_LIST;
        return 0;
    }

    /* BROADCAST */
    if (strcmp(cmd, "/BROADCAST") == 0) {
        const char *msg = strstr(p, " ");
        if (!msg) return -1;

        msg = skip_spaces(msg);
        if (*msg == '\0') return -1;

        out->type = PARSE_BROADCAST;
        copy_token(out->arg2, sizeof(out->arg2), msg);
        return 0;
    }

    /* MENSAJE DIRECTO */
    if (strcmp(cmd, "/MSG") == 0) {
        char user[32] = {0};
        char message[957] = {0};

        if (sscanf(p, "/msg %31s %956[^\n]", user, message) < 2 &&
            sscanf(p, "/MSG %31s %956[^\n]", user, message) < 2) {
            return -1;
        }

        out->type = PARSE_DIRECT;
        copy_token(out->arg1, sizeof(out->arg1), user);
        copy_token(out->arg2, sizeof(out->arg2), message);
        return 0;
    }

    /* STATUS */
    if (strcmp(cmd, "/STATUS") == 0) {
        char status[16] = {0};

        if (sscanf(p, "/status %15s", status) != 1 &&
            sscanf(p, "/STATUS %15s", status) != 1) {
            return -1;
        }

        /* Normalizar estado */
        to_uppercase(status);

        if (strcmp(status, "ACTIVO") == 0 || strcmp(status, "ACTIVE") == 0) {
            strcpy(status, "ACTIVO");
        } else if (strcmp(status, "OCUPADO") == 0 || strcmp(status, "BUSY") == 0) {
            strcpy(status, "OCUPADO");
        } else if (strcmp(status, "INACTIVO") == 0 || strcmp(status, "INACTIVE") == 0) {
            strcpy(status, "INACTIVO");
        } else {
            return -1;
        }

        out->type = PARSE_STATUS;
        copy_token(out->arg1, sizeof(out->arg1), status);
        return 0;
    }

    /* INFO */
    if (strcmp(cmd, "/INFO") == 0) {
        char user[32] = {0};

        if (sscanf(p, "/info %31s", user) != 1 &&
            sscanf(p, "/INFO %31s", user) != 1) {
            return -1;
        }

        out->type = PARSE_INFO;
        copy_token(out->arg1, sizeof(out->arg1), user);
        return 0;
    }

    /* Comando no válido */
    return -1;
}