#ifndef PROTOCOLO_H
#define PROTOCOLO_H

/*
 * PROTOCOLO DE COMUNICACION (TEXTO PLANO)
 * ---------------------------------------
 * Este archivo define el formato oficial de mensajes entre cliente y servidor.
 *
 * FORMATO GENERAL:
 *   COMANDO|arg1|arg2|...\n
 *
 * Todos los mensajes:
 *   - Son texto plano (ASCII)
 *   - Usan '|' como separador
 *   - Terminan con '\n'
 *
 * Ejemplos:
 *   REGISTER|juan\n
 *   BROADCAST|hola a todos\n
 *   DIRECT|maria|hola\n
 *   LIST\n
 *   INFO|juan\n
 *   STATUS|ACTIVO\n
 *   EXIT\n
 */

/* ================= COMANDOS CLIENTE -> SERVIDOR ================= */

#define CMD_REGISTER   "REGISTER"
#define CMD_BROADCAST  "BROADCAST"
#define CMD_DIRECT     "DIRECT"
#define CMD_LIST       "LIST"
#define CMD_INFO       "INFO"
#define CMD_STATUS     "STATUS"
#define CMD_EXIT       "EXIT"

/* ================= RESPUESTAS SERVIDOR -> CLIENTE ================= */

#define RESP_OK            "OK"
#define RESP_ERROR         "ERROR"
#define RESP_MSG           "MSG"
#define RESP_LIST          "LIST"
#define RESP_INFO          "INFO"
#define RESP_DISCONNECTED  "DISCONNECTED"

/*
 * Formatos esperados:
 *
 * OK:
 *   OK|mensaje\n
 *
 * ERROR:
 *   ERROR|mensaje\n
 *
 * MENSAJE:
 *   MSG|emisor|destino|mensaje\n
 *
 * LISTA:
 *   LIST|usuario1,ACTIVO;usuario2,OCUPADO\n
 *
 * INFO:
 *   INFO|ip,estado\n
 *
 * DESCONECTADO:
 *   DISCONNECTED|usuario\n
 */

/* ================= STATUS ================= */

#define STATUS_ACTIVO    "ACTIVO"
#define STATUS_OCUPADO   "OCUPADO"
#define STATUS_INACTIVO  "INACTIVO"

/* ================= CONFIG ================= */

#define MAX_MSG_LEN 1024
#define INACTIVITY_TIMEOUT 20

#endif