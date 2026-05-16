/* include/network.h */
#ifndef NETWORK_H
#define NETWORK_H

// Crea y configura un socket UNIX en la ruta especificada
// Retorna el file descriptor (fd) o -1 si falla
int create_unix_socket(const char *path);

#endif