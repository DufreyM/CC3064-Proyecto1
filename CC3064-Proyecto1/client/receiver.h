/**
 * ============================================================
 * RECEPTOR DE MENSAJES - INTERFAZ
 * Archivo: receiver.h
 *
 * Declara el hilo encargado de recibir mensajes del servidor
 * de forma asíncrona en el cliente.
 * ============================================================
 */

#ifndef RECEIVER_H
#define RECEIVER_H

/* Hilo que escucha el socket y procesa mensajes del servidor */
void *receiver_thread(void *arg);

#endif