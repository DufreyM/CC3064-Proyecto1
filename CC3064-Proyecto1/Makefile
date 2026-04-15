# ============================================================
# MAKEFILE - SISTEMA DE MENSAJERÍA
# Compila cliente y servidor usando gcc y pthreads
# ============================================================

CC=gcc
CFLAGS=-Wall -Wextra -Werror -std=c11 -O2 -pthread

SERVER_SRC=server/server.c server/session.c server/client_manager.c
CLIENT_SRC=client/client.c client/receiver.c client/command_parser.c

# Compila ambos ejecutables
all: servidor cliente

# Compila el servidor
servidor: $(SERVER_SRC) protocolo.h
	$(CC) $(CFLAGS) -o servidor $(SERVER_SRC)

# Compila el cliente
cliente: $(CLIENT_SRC) protocolo.h
	$(CC) $(CFLAGS) -o cliente $(CLIENT_SRC)

# Limpia ejecutables generados
clean:
	rm -f servidor cliente

.PHONY: all clean