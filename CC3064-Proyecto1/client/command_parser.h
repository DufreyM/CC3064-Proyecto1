/**
 * ============================================================
 * PARSER DE COMANDOS - INTERFAZ
 * Archivo: command_parser.h
 *
 * Define los tipos y estructuras usadas para representar
 * comandos ingresados por el usuario en el cliente.
 * ============================================================
 */

#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

/* Tipos de comandos soportados por el cliente */
typedef enum {
    PARSE_INVALID = 0,
    PARSE_HELP,
    PARSE_EXIT,
    PARSE_BROADCAST,
    PARSE_DIRECT,
    PARSE_STATUS,
    PARSE_LIST,
    PARSE_INFO
} ParseType;

/* Resultado del parseo: tipo de comando + argumentos */
typedef struct {
    ParseType type;
    char arg1[32];   /* usuario o estado */
    char arg2[957];  /* mensaje */
} ParsedCommand;

/* Convierte una línea de texto en un comando estructurado */
int parse_input_line(const char *line, ParsedCommand *out);

#endif