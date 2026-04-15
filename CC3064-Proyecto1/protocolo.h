/**
 * ============================================================
 * PROTOCOLO DE COMUNICACIÓN
 * Archivo: protocolo.h
 *
 * Define el formato de mensajes entre cliente y servidor.
 * Se basa en texto plano con separador '|' y terminación '\n'.
 *
 * Formato general:
 *   COMANDO|arg1|arg2|...\n
 * ============================================================
 */

#ifndef PROTOCOLO_H
#define PROTOCOLO_H

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

/* ================= FORMATOS PRINCIPALES ================= */
/*
 * OK:            OK|mensaje\n
 * ERROR:         ERROR|mensaje\n
 * MENSAJE:       MSG|emisor|destino|mensaje\n
 * LISTA:         LIST|user,status;...\n
 * INFO:          INFO|ip,status\n
 * DESCONECTADO:  DISCONNECTED|usuario\n
 */

/* ================= STATUS ================= */

#define STATUS_ACTIVO    "ACTIVO"
#define STATUS_OCUPADO   "OCUPADO"
#define STATUS_INACTIVO  "INACTIVO"

/* ================= CONFIG ================= */

#define MAX_MSG_LEN 1024
#define INACTIVITY_TIMEOUT 20

#endif